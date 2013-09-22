#ifndef AIO_COMMON_UTILITY_FILE_MAPPING_HEAP_H
#define AIO_COMMON_UTILITY_FILE_MAPPING_HEAP_H

#include <xirang/memory.h>
#include <xirang/string.h>
#include <xirang/io.h>

namespace aio
{
	struct file_mapping_heap_imp;
	struct file_mapping_heap_info
	{
		long_size_t map_space_size;
		long_size_t map_file_size;

		long_size_t total_view_size;
		long_size_t outer_free_size;
		long_size_t inner_free_size;
		std::size_t m_view_align, m_view_size;
		long_size_t soft_limit, hard_limit;
	};
	struct AIO_COMM_API file_mapping_heap : ext_heap
	{
	public:
		typedef iref<io::read_map, io::write_map, io::ioctrl, io::ioinfo> ar_type;

		file_mapping_heap(const ar_type& file, heap* hp, memory::thread_policy thp);
		~file_mapping_heap();

	public: //heap methods

		virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint );

		virtual void free(void* p, std::size_t size, std::size_t alignment );

		/// \return the underling ext_heap, can be null.
		virtual heap* underling();

		virtual bool equal_to(const heap& rhs) const ;

		/// get hook
		virtual heap* hook();

		/// set hook
		/// \return previous hook
		virtual heap* hook(heap* newhook);

	public:

		/// allocate a block in external heap
		virtual handle allocate(std::size_t size, std::size_t alignment, handle hint) ;

		/// release an external block
		virtual void deallocate(handle p) ;

		/// map a block into memory, ref count internally.
		virtual void* track_pin(handle h) ;
		/// map a block into memory, ref count the view only
		virtual void* pin(handle h) ;

		virtual int track_pin_count(handle h) const;
		virtual int view_pin_count(handle h) const;

		/// unmap a block. 
		virtual int track_unpin(void* h);
		/// unmap a block. 
		virtual int unpin(void* h);

		/// TODO: is it necessary?
		/// write to external block directly. if h have been mapped into memory, update the memory.
		virtual std::size_t write(handle h, const void* src, std::size_t n);

		/// TODO: is it necessary?
		/// read from external block directly. if the block has been mapped, read the memory block.
		virtual std::size_t read(handle, void* dest, std::size_t);

		/// sync the memory to external, if h is invalid, sync all. 
		virtual void sync(handle h);

		virtual void set_limit(std::size_t soft, std::size_t hard);

		virtual void pack();

		virtual const file_mapping_heap_info& get_info() const;
	private:
		file_mapping_heap_imp* m_imp;
	};
}

#endif //end AIO_COMMON_UTILITY_FILE_MAPPING_HEAP_H
