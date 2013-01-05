#include <aio/utility/ext1/file_mapping_heap.h>
#include <aio/common/atomic.h>
#include <aio/common/memory.h>
#include <aio/common/archive/mem_archive.h>


//STL
#include <set>
#include <unordered_map>
#include <vector>
#include <cstring>	//for memcpy

#ifndef MSVC_COMPILER_
#include <unistd.h>
#else
#include <cstdint>
#include <allocators>
#endif


//BOOST
#include <boost/bimap.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <fstream>

#if 0
//DEBUG
#include <iostream>
#endif

const uint32_t sig_ext_heap = 0x48545845; //"EXTH"

namespace aio
{
	using std::memcpy;

	AIO_EXCEPTION_TYPE(bad_ext_heap_format);

	using boost::numeric_cast;

	std::size_t round_size(std::size_t s)
	{
		std::size_t tail = s & 0xf;
		if (tail == 0)
			return s;
		return s - tail + 0x10; 
	}

	struct file_mapping_heap_imp
	{
		typedef int pin_counter;
		typedef ext_heap::handle handle;
		typedef boost::bimap<handle, byte*, boost::bimaps::with_info<int> > view_map_type;

		file_mapping_heap_imp(const file_mapping_heap::ar_type& file, heap* hp, memory::thread_policy thp)
			: m_heap(hp), m_thp(thp), m_map_file(file)
		{
			info.map_file_size  = 0;
			info.map_space_size = 0;
			info.total_view_size = 0;
			info.outer_free_size = 0;
			info.inner_free_size = 0;
			info.m_view_align = 0x10000;
			info.m_view_size = 1024*1024;

			info.soft_limit = 0;
			info.hard_limit = 0;

			if (file.get<io::read_map>().size() != 0){
				load_();
			}
		}

		~file_mapping_heap_imp()
		{
			AIO_PRE_CONDITION(pin_map.empty());
			AIO_PRE_CONDITION(no_pinned_memory());
			unload_();
		}

		// try to allocate the free space from inner memory
		// if failed, then allocate from outer
		handle allocate(std::size_t size, std::size_t alignment, handle hint) 
		{

			handle h = allocate_free_space_(inner_free_(), size, alignment, hint);
			if (!h.empty())
			{
				info.inner_free_size -= h.size();
				return h;
			}

			h = allocate_free_space_(outer_free_(), size, alignment, hint);
			if (!h.empty())
			{
				info.outer_free_size -= h.size();
				return h;
			}

			// if there are no enough space in both inner and outer memory
			// expend a space in outer
			handle expand_space = new_space_(size);
			handle r (expand_space.begin(), expand_space.begin() + size);
			if (expand_space.size() > size)
			{
				expand_space = handle(expand_space.begin() + size, expand_space.end());
				info.outer_free_size += expand_space.size();
				outer_free_().insert(expand_space);
			}
			return r;
		}

		// pre: h is unpinned
		// insert the h to free space
		void deallocate(handle h)
		{
			AIO_PRE_CONDITION(track_pin_count(h) == 0);

			handle r = h;
			view_map_type::left_iterator view = locate_in_view_map_(h.begin());
			if (view != view_map.left.end())	// in memory
			{
				std::set<handle >::iterator in_pos = inner_free_().lower_bound(h);	// h is never in any free_space
				if (in_pos != inner_free_().end()
						&& h.end() == in_pos->begin()	// right upon pos
						&& view->first.end() >= in_pos->end())//contains in same view block
				{
					r = handle(r.begin(), in_pos->end());
					inner_free_().erase(in_pos++);
				}
				if (in_pos != inner_free_().begin())
				{
					--in_pos;

					if ( in_pos->end() == h.begin()	//left upon
							&& view->first.begin() <= in_pos->begin()) //contains the range in memory view
					{
						r = handle(in_pos->begin(), r.end());
						inner_free_().erase(in_pos);
					}
				}

				inner_free_().insert(r);
				info.inner_free_size += h.size();
				return;
			}

			// not in memory
			return_to_outer_(h);
		}


		long_offset_t get_handle(const byte* ap) const
		{
			byte*p = (byte*)ap;
            view_map_type::right_const_iterator itr = view_map.right.lower_bound(p);
			if (itr != view_map.right.end() 
					&& itr->first == p)
				return itr->second.begin();

			AIO_PRE_CONDITION (itr != view_map.right.begin());	// fail if found!
			--itr;
			AIO_PRE_CONDITION (itr->info > 0 );	//view counter != 0
			byte* p_start = itr->first;
			byte* p_end = itr->first + itr->second.size();
			AIO_PRE_CONDITION (p >= p_start && p < p_end);
			return itr->second.begin() + (p - p_start);
		}

		byte* pin(handle h)
		{
#ifndef NDEBUG			
			return pin_(h, true);
#else
			return pin_(h, false);
#endif				
		}

		byte* track_pin(handle h)
		{
			return pin_(h, true);
		}

		int track_pin_count(handle h) const
		{
			std::unordered_map<long_offset_t, pin_counter>::const_iterator itr = pin_map.find(h.begin());
			return itr == pin_map.end() ? 0 : itr->second;

		}

		int view_pin_count(handle h) const
		{
			view_map_type::left_iterator view = const_cast<file_mapping_heap_imp*>(this)->locate_in_view_map_(h.begin());
			if (view != view_map.left.end())	// in memory
			{
				return view->info;
			}
			return 0;
		}

		int track_unpin(byte* p)
		{
			return unpin_(p, true);
		}

		int unpin(byte* p)
		{
#ifndef NDEBUG
			return unpin_(p, true);
#else
			return unpin_(p, false);
#endif
		}
		void pack()
		{
			std::vector <handle> empty_views;
			for(view_map_type::left_const_iterator itr = view_map.left.begin(); itr != view_map.left.end(); ++itr)
				if (itr->info == 0)
					empty_views.push_back(itr->first);
			for (std::vector <handle>::iterator itr = empty_views.begin(); itr != empty_views.end(); ++itr)
			{
				view_map_type::left_iterator  pos = view_map.left.find(*itr);
				return_to_outer_(pos->first);

				std::unordered_map<long_offset_t, io::write_view*>::iterator itr_view = view_regions.find(pos->first.begin());
				AIO_PRE_CONDITION(itr_view != view_regions.end());
				delete itr_view->second;
				view_regions.erase(itr_view);

				info.total_view_size -= pos->first.size();
				info.inner_free_size -= pos->first.size();
				AIO_PRE_CONDITION(inner_free_().count(pos->first) == 1);
				inner_free_().erase(pos->first);
				view_map.left.erase(pos);
			}
		}

		void sync() {
			writer_().sync();
		}

		private:
		// Find free in given space
		// First Find First Fit algorithm
		handle allocate_free_space_(std::set<handle>& space, std::size_t size, std::size_t /*alignment*/, handle) 
		{
			for (std::set<handle>::iterator itr = space.begin(); itr != space.end(); ++itr)
			{
				if (itr->size() >= size) //found
				{
					handle can_use = *itr;
					space.erase(itr);

					handle h (can_use.begin(), can_use.begin() + size);
					if (can_use.size() > size)
					{
						can_use = handle(can_use.begin() + size, can_use.end());
						space.insert(can_use);
					}
					return h;
				}
			}
			return handle();
		}

		// locate the view which contains the pos h
		view_map_type::left_iterator locate_in_view_map_(long_offset_t h) 
		{
			handle hm (h, std::numeric_limits<long_offset_t>::max());
			view_map_type::left_iterator pos = view_map.left.lower_bound(hm);
			if (pos != view_map.left.begin())
			{
				--pos;
				if (pos->first.end() > h)
					return pos;
			}
			return view_map.left.end();
		}

		std::set<handle >::iterator locate_in_outer_space_(long_offset_t h) const
		{
			handle hm (h, std::numeric_limits<long_offset_t>::max());
			std::set<handle >::const_iterator pos = outer_free_().lower_bound(hm);
			if (pos != outer_free_().begin())
			{
				std::set<handle >::const_iterator itr = pos;
				--itr;
				if (itr->end() > h)
					return itr;
			}
			return pos;
		}

		void return_to_outer_(handle h)
		{
			handle r = h;
			std::set<handle >::const_iterator out_pos = outer_free_().lower_bound(h);
			if (out_pos != outer_free_().end()
					&& h.end() == out_pos->begin())	// right upon
			{
				r = handle(r.begin(), out_pos->end());
				outer_free_().erase(out_pos++);
			}
			if (out_pos != outer_free_().begin())
			{
				--out_pos;

				if ( out_pos->end() == h.begin())	//left upon
				{
					r = handle(out_pos->begin(), r.end());
					outer_free_().erase(out_pos);
				}
			}

			outer_free_().insert(r);
			info.outer_free_size += h.size();
		}

		byte* pin_(handle h, bool track)
		{
			view_map_type::left_iterator vpos = locate_in_view_map_(h.begin());
			if (vpos != view_map.left.end())
			{
				++vpos->info;
				if (track)
					++pin_map[h.begin()];
				//info.inner_free_size -= h.size();
				return vpos->second + (h.begin() - vpos->first.begin());
			}

			// not in exists view.
			return new_view_(h, track);
		}

		byte * new_view_(handle h, bool track)
		{
			handle ha = h;
			std::size_t view_align = info.m_view_align;
			if (ha.begin() % view_align != 0)
			{
				ha = handle(ha.begin() - ha.begin() % info.m_view_align, ha.end());
			}

			long_size_t view_size = ha.size() > info.m_view_size ? ha.size() : info.m_view_size;
			if (view_size % info.m_view_align != 0)
			{
				view_size += info.m_view_align - view_size % info.m_view_align;
			}
			ha = handle(ha.begin(), ha.begin() + view_size);

			auto p_view = create_view_(ha);
			view_map.left.insert(std::make_pair(ha, p_view.begin()));
			info.inner_free_size += ha.size() - h.size();
			view_map.left.find(ha)->info = 1;

			if (track)
				pin_map[h.begin()] = 1;

			byte *p = p_view.begin() + (h.begin() - ha.begin());

			// move the view space from outer to inner 
			//
			std::set<handle >::iterator lower = outer_free_().lower_bound(handle(ha.begin(), ha.begin()));
			std::set<handle >::iterator upper = outer_free_().lower_bound(handle(ha.end(), ha.end()));

			if (lower != outer_free_().end())
			{
				if (lower->begin() < ha.end())
				{
					handle h = *lower;
					if (h.end() >= ha.end())
					{
						outer_free_().erase(lower++);
						outer_free_().insert(handle(ha.end(), h.end()));
					}
				}
			}

			inner_free_().insert(lower, upper);
			outer_free_().erase(lower, upper);

			if (info.map_space_size < numeric_cast<long_size_t>(ha.end()))
			{
				inner_free_().insert(handle(info.map_space_size, ha.end()));
				info.map_space_size = ha.end();

			}

			return p;
		}

		int unpin_(byte* p, bool track)
		{
			view_map_type::right_iterator  itr = view_map.right.lower_bound(p);
			if (itr == view_map.right.end() 
					|| itr->first != p)
			{

				AIO_PRE_CONDITION (itr != view_map.right.begin());	// fail if found!
				--itr;
				AIO_PRE_CONDITION (itr->info > 0 );	//view counter != 0
			}
			--itr->info;

			if(track)
			{

				byte* p_start = itr->first;
				byte * p_end = itr->first + itr->second.size();
				AIO_PRE_CONDITION (p >= p_start && p < p_end);

				long_offset_t h_off =  itr->second.begin() + (p - p_start);
				std::unordered_map<long_offset_t, pin_counter>::iterator pos = pin_map.find(h_off);
				AIO_PRE_CONDITION(pos != pin_map.end());
				if (--pos->second == 0)
				{
					pin_map.erase(pos);
					return 0;
				}
				return pos->second;
			}
			return itr->info;
		}

		range<byte*> create_view_(handle h)
		{
			//if soft_limit reached, it'll try to recycle the empty map view
			if (info.total_view_size   > info.soft_limit)
			{
				pack();
			}
			//if hard_limit reached, it'll failed,throw and exception
			if (info.total_view_size + h.size() > info.hard_limit && info.hard_limit > info.soft_limit)
			{
				throw std::bad_alloc();
			}

			if (info.map_file_size < numeric_cast<long_size_t>(h.end()))
			{
				ioctrl_().truncate(h.end());
				info.map_file_size = h.end();
			}
			aio::unique_ptr<io::write_view> region = writer_().view_wr(h);
			view_regions[h.begin()] = region.get();
			auto addr = region->address();
			region.release();
			info.total_view_size += h.size();
			return addr;
		}

		handle new_space_(std::size_t n)
		{
			long_size_t new_space_size = info.map_space_size << 1;
			if (new_space_size < n + info.map_space_size)
				new_space_size = n + info.map_space_size;

			handle r( info.map_space_size, new_space_size - info.map_space_size	);

			info.map_space_size = new_space_size;
			return r;
		}

		std::set<handle >& inner_free_() { return free_space[0];}
		std::set<handle >& outer_free_() { return free_space[1];}
		const std::set<handle >& inner_free_() const { return free_space[0];}
		const std::set<handle >& outer_free_() const { return free_space[1];}

		io::read_map& reader_() const { return m_map_file.get<io::read_map>();}
		io::write_map& writer_() const { return m_map_file.get<io::write_map>();}
		io::ioctrl& ioctrl_() const { return m_map_file.get<io::ioctrl>();}
		io::ioinfo& ioinfo_() const { return m_map_file.get<io::ioinfo>();}

		bool no_pinned_memory() const{
			bool result = true;
			for (view_map_type::left_const_iterator  itr = view_map.left.begin(); itr != view_map.left.end(); ++itr)
			{
				result = result && (itr->info == 0);
				AIO_PRE_CONDITION(result);
			}
			return result;
		}

		void load_() {
			io::ioinfo& arinfo = ioinfo_();
			if (arinfo.size() == 0)
				return;
			
			std::size_t tail_size = sizeof(long_offset_t) + sizeof(uint32_t);
			if (arinfo.size() < tail_size)
				AIO_THROW(bad_ext_heap_format);
			auto tail_view = reader_().view_rd(ext_heap::handle(arinfo.size() - tail_size, arinfo.size()));
			const uint32_t& sig = *reinterpret_cast<const uint32_t*>(tail_view->address().begin());
			const long_offset_t& end_pos = *reinterpret_cast<const long_offset_t*>(tail_view->address().begin() + sizeof(uint32_t));
			if (sig != sig_ext_heap)
				AIO_THROW(bad_ext_heap_format);
			auto manager_view = reader_().view_rd(ext_heap::handle(end_pos, arinfo.size() - tail_size));
			buffer<byte> buf(manager_view->address());

			io::buffer_in mar(buf);
			iref<io::reader> ird(mar);
			size_t items = mar.data().size() /sizeof(handle);
			mar.seek(0);
			while(items--)
			{
				using namespace sio;
				long_offset_t b, e;
				ird.get<io::reader>() & b & e;
				handle h(b, e);
				free_space[1].insert(h);
				info.outer_free_size += h.size() ;
			}

			handle tail_block(end_pos, arinfo.size());
			return_to_outer_(tail_block);

			info.outer_free_size += tail_block.size() ;
			info.map_file_size  = arinfo.size();
			info.map_space_size = arinfo.size();
		}

		void unload_(){
			view_map.clear();

			for (std::unordered_map<long_offset_t, io::write_view*>::iterator itr = view_regions.begin(); itr != view_regions.end(); ++itr)
				delete itr->second;
			view_regions.clear();

			long_offset_t end_pos = info.map_file_size;

			typedef std::reverse_iterator<std::set<handle>::iterator> iterator;
			free_space[1].insert(free_space[0].begin(), free_space[0].end());
			free_space[0].clear();

			iterator rbeg(free_space[1].end()), rend(free_space[1].begin());
			for (; rbeg != rend; ++rbeg)
			{
				if (rbeg->end() == end_pos)
					end_pos = rbeg->begin();
				else
					break;
			}

			io::mem_archive war;
			iref<io::writer> ar(war);
			
			using namespace sio;
			std::set<handle >::iterator use_end = rbeg.base();
			for (std::set<handle >::iterator itr = free_space[1].begin(); itr != use_end; ++itr)
			{
				ar.get<io::writer>() & itr->begin() & itr->end();
			}
			ar.get<io::writer>() & sig_ext_heap & end_pos;

			free_space[1].clear();

			ioctrl_().truncate(numeric_cast<long_size_t>(end_pos) + war.size());
			auto manager_view = writer_().view_wr(ext_heap::handle(end_pos, end_pos + war.size()));
			std::copy(war.data().begin(), war.data().end(), manager_view->address().begin());
		}

#if 0
		virtual void  dump() const { 
			std::cerr << "\ninner:\n";

			for (auto itr = free_space[0].begin(); itr != free_space[0].end(); ++itr)
			{
				std::cerr << "(" << itr->begin() << ", " << itr->end() << ")";
			}
			std::cout << "\nouter:\n";
			for (auto itr = free_space[1].begin(); itr != free_space[1].end(); ++itr)
			{
				std::cerr << "(" << itr->begin() << ", " << itr->end() << ")";
			}

		}
#endif
		public:
		heap* m_heap;
		memory::thread_policy m_thp;

		file_mapping_heap::ar_type m_map_file;

		file_mapping_heap_info info;

		std::set<handle > free_space[2];
		view_map_type view_map;

		// pinned handle, track/debug only
        std::unordered_map<long_offset_t, pin_counter> pin_map;

		//platform data
		std::unordered_map<long_offset_t, io::write_view*> view_regions;
#ifndef MSVC_COMPILER_
		//TODO for mac std::mutex mutex;
#else
        ::stdext::threads::mutex mutex;
#endif
	};

	file_mapping_heap::file_mapping_heap(const file_mapping_heap::ar_type& file, heap* hp, memory::thread_policy thp)
		: m_imp(new file_mapping_heap_imp(file, hp, thp))
	{
	}

	file_mapping_heap::~file_mapping_heap()
	{
		delete m_imp;
	}

	void* file_mapping_heap::malloc(std::size_t size, std::size_t alignment, const void* hint )
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif
		size = round_size(size);

		handle h = m_imp->allocate(size, alignment, handle());

		return m_imp->pin(h);
	}

	void file_mapping_heap::free(void* p, std::size_t size, std::size_t alignment )
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif
		size = round_size(size);

		auto beg = m_imp->get_handle(reinterpret_cast<byte*>(p));
		handle h(beg, beg + size);
		m_imp->unpin(reinterpret_cast<byte*>(p));
		m_imp->deallocate(h);
	}

	/// \return the underling ext_heap, can be null.
	heap* file_mapping_heap::underling() { return m_imp->m_heap;}

	bool file_mapping_heap::equal_to(const heap& rhs) const 
	{
		return this == &rhs;
	}

	/// get hook
	heap* file_mapping_heap::hook() { return 0;}

	/// set hook
	/// \return previous hook
	heap* file_mapping_heap::hook(heap* newhook) { return 0;}


	/// allocate a block in external heap
	ext_heap::handle file_mapping_heap::allocate(std::size_t size, std::size_t alignment, handle hint) 
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif

		size = round_size(size);
		return m_imp->allocate(size, alignment, hint);
	}

	/// release an external block
	void file_mapping_heap::deallocate(handle p) 
	{
#ifndef MSVC_COMPILER_
		//todo for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif
		m_imp->deallocate(p);
	}

	/// map a block into memory, ref count internally.
	void* file_mapping_heap::track_pin(handle h) 
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif
		return reinterpret_cast<void*>(m_imp->track_pin(h));
	}

	/// map a block into memory, ref count the view only.
	void* file_mapping_heap::pin(handle h) 
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif

		return reinterpret_cast<void*>(m_imp->pin(h));
	}

	int file_mapping_heap::track_pin_count(handle h) const
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif

		return m_imp->track_pin_count(h);
	}

	int file_mapping_heap::view_pin_count(handle h) const
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif

		return m_imp->view_pin_count(h);
	}

	/// unmap a block. 
	int file_mapping_heap::track_unpin(void* h)
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif

		return m_imp->track_unpin(reinterpret_cast<byte*>(h));
	}

	/// unmap a block. 
	int file_mapping_heap::unpin(void* h)
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif

		return m_imp->unpin(reinterpret_cast<byte*>(h));
	}

	/// TODO: is it necessary?
	/// write to external block directly. if h have been mapped into memory, update the memory.
	std::size_t file_mapping_heap::write(handle h, const void* src, std::size_t n)
	{
#ifndef MSVC_COMPILER_
		//TODO for mac td::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif

		byte* p = m_imp->pin(h);
		memcpy(p, src, n);
		m_imp->unpin(p);

		return n;
	}

	/// TODO: is it necessary?
	/// read from external block directly. if the block has been mapped, read the memory block.
	std::size_t file_mapping_heap::read(handle h, void* dest, std::size_t n)
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif

		byte* p = m_imp->pin(h);
		memcpy(dest, p, n);
		m_imp->unpin(p);

		return n;
	}

	/// sync the memory to external, if h is invalid, sync all. 
	void file_mapping_heap::sync(handle h)
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif
		m_imp->sync();
	}
	void file_mapping_heap::set_limit(std::size_t soft, std::size_t hard)
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif
		m_imp->info.soft_limit = soft;
		m_imp->info.hard_limit = hard;
	}

	void file_mapping_heap::pack()
	{
#ifndef MSVC_COMPILER_
		//TODO for mac std::lock_guard<std::mutex> lock(m_imp->mutex);
#else
		stdext::threads::_Scoped_lock lock(m_imp->mutex);
#endif
		m_imp->pack();
	}

	const file_mapping_heap_info& file_mapping_heap::get_info() const
	{
		return m_imp->info;
	}
}

