#include <xirang/zip.h>
#include <xirang/io/memory.h>
#include <xirang/io/exchs11n.h>
#include <xirang/io/adaptor.h>
#include <xirang/deflate.h>

#include <map>
#include <ctime>

namespace xirang{ namespace zip{
	namespace {
		const uint16_t K_fix_part_of_local_header = 30;
		const uint16_t K_fix_part_of_central_header = 46;
		const uint32_t K_sig_local_eader = 0x04034b50;
		const uint32_t K_sig_end_central_dir_signature = 0x06054b50;
		const uint32_t K_sig_central_file_header = 0x02014b50;
		const uint32_t K_sig_zip64_end_of_cd_locator = 0x07064b50;
		const uint32_t K_sig_zip64_end_cd = 0x06064b50;

		const uint32_t K_sizeof_zip64_end_cd = 56;
		const long_size_t K_sizeof_zip64_locator = 20;
		const uint16_t K_sizeof_zip64_extra_field = 32;
		const uint16_t K_sizeof_cd_end = 22;

		const long_size_t K_max_zip_head_size = 64 * 1024;
		const long_size_t K_cd_end_size = K_max_zip_head_size + K_sizeof_zip64_locator;	// 64K + 20

		const uint16_t K_z64_extra_id = 1;
		void setDateTime(file_header& h)
		{
			std::time_t rawtime = time(0);
			struct std::tm* ptm = localtime(&rawtime);
			h.modified_date = ((ptm->tm_year - 80) << 9)  + ((ptm->tm_mon + 1) << 5) + ptm->tm_mday;
			h.modified_time = (ptm->tm_hour << 11) + (ptm->tm_min << 5) + (ptm->tm_sec >> 1);
		}
	}

	class zip_package_imp{
	public:
		std::map<file_path, file_header, path_less> items;

		io::read_map* package;

		zip_package_imp(io::read_map& pkg) : package(&pkg){
			if (pkg.size() == 0)
				return;
			//load cd
			//1. load cd end
			auto size = std::min(K_max_zip_head_size, package->size());
			auto view = package->view_rd(ext_heap::handle(package->size() - size, package->size()));
			auto address = view.get<io::read_view>().address();

			// reverse find the signature
			typedef std::reverse_iterator<const byte*> iterator;
			iterator rbeg(address.end()), rend(address.begin());

			const byte* sig_beg = reinterpret_cast<const byte*>(&K_sig_end_central_dir_signature);
			const byte* sig_end = sig_beg + sizeof(K_sig_end_central_dir_signature);

			iterator rsbeg(sig_end), rsend(sig_beg);
			iterator pos = std::search(rbeg, rend, rsbeg, rsend);
			if (pos == rend) //not found
				AIO_THROW(data_corrupted_exception)("end central dir signature is not found");

			//load offset info
			io::buffer_in  mrd(address);
			mrd.seek(pos.base() - address.begin());
			auto loader = io::exchange::as_source(mrd);
			loader // skip "number of this disk: "
				// "number of central start disk: " 
				// "number of entries on this disk: " 
				// "total number of entries"
				& io::skip_n<2 + 2 + 2 + 2>();
			long_size_t zip_cd_size = io::load<uint32_t>(loader);
			long_size_t zip_cd_offset = io::load<uint32_t>(loader);

			if (zip_cd_offset == uint32_t(-1) 
					|| zip_cd_size == uint32_t(-1)){	//zip 64
				//read Zip64 end of central directory locator
				if (pos.base() < address.begin() + K_sizeof_zip64_locator)
					AIO_THROW(data_corrupted_exception)("Failed to locate Zip64 end of central directory locator");
				mrd.seek(pos.base() - address.begin() - K_sizeof_zip64_locator);

				if (io::load<uint32_t>(loader) != K_sig_zip64_end_of_cd_locator)
					AIO_THROW(data_corrupted_exception)("Failed to read Zip64 end of central directory locator");

				loader & io::skip_t<uint32_t>(); //4B number of the disk with the start of the zip64 end of central directory      
				uint64_t zip64_end_cd_offset = io::load<uint32_t>(loader);

				auto z64_end_view = package->view_rd(ext_heap::handle(zip64_end_cd_offset, zip64_end_cd_offset + K_sizeof_zip64_end_cd));
				auto z64_cd_address = z64_end_view.get<io::read_view>().address();
				io::buffer_in  z64_cd_mrd(z64_cd_address);
				auto z64_cd_loader = io::exchange::as_source(z64_cd_mrd);

				if (io::load<uint32_t>(z64_cd_loader) != K_sig_zip64_end_cd)
					AIO_THROW(data_corrupted_exception)("Zip64 end of central directory signature error");
				z64_cd_loader & io::skip_n<8 	// size of zip64 end of central directory record
						+ 2 + 2 	//version made by & version needed to extract
						+ 4 		// number of this disk 
						+ 4			//number of the disk with the start of the central directory
						+ 8			// total number of entries in the central directory on this disk
						+ 8			// total number of entries in the central directory
						>();
				zip_cd_offset = io::load<uint64_t>(z64_cd_loader);
				zip_cd_size = io::load<uint64_t>(z64_cd_loader);
			}

			view = package->view_rd(ext_heap::handle(zip_cd_offset, zip_cd_offset + zip_cd_size));
			address = view.get<io::read_view>().address();
			io::buffer_in  cdrd(address);
			auto cdin = io::exchange::as_source(cdrd);
			while(cdrd.readable())
			{
				file_header h;
				h.package = package;
				if (io::load<uint32_t>(cdin) != K_sig_central_file_header) 
					AIO_THROW(data_corrupted_exception)("central file header signature error");

				uint16_t name_len(0), extra_len(0), comments_len(0);
				cdin & h.creator_version
					& h.reader_version
					& h.flags
					& h.method
					& h.modified_time
					& h.modified_date
					& h.crc32;
				h.compressed_size = io::load<uint32_t>(cdin);
				h.uncompressed_size = io::load<uint32_t>(cdin);
				cdin & name_len
					& extra_len
					& comments_len
					& io::skip_n<2 + 2>()	//2: disk number start
										//4: internal file attributes
					& h.external_attrs;
				h.relative_offset_ = io::load<uint32_t>(cdin);
				auto var_off = address.begin() + cdrd.offset();
				h.name = file_path(const_range_string(reinterpret_cast<const char*>(
								var_off), name_len));
				var_off += name_len;
				h.extra.assign(make_range(var_off, var_off + extra_len));
				var_off += extra_len;

				h.comments = const_range_string(reinterpret_cast<const char*>(var_off), comments_len);
				cdrd.seek(cdrd.offset() + name_len + extra_len + comments_len);

				if (h.compressed_size == uint32_t(-1) 
						|| h.uncompressed_size == uint32_t(-1)
						|| h.relative_offset_ == uint32_t(-1)){
					io::buffer_in exin(h.extra);
					while (exin.readable()){
						auto exdes = io::exchange::as_source(exin);
						uint16_t id = io::load<uint16_t>(exdes);
						uint16_t dsize = io::load<uint16_t>(exdes);
						if (id == K_z64_extra_id){
							h.uncompressed_size = io::load<uint64_t>(exdes);
							h.compressed_size = io::load<uint64_t>(exdes);
							h.relative_offset_ = io::load<uint64_t>(exdes);
							exdes & io::skip_n<4>(); 	//skip Disk Start Number
						}
						else
							exin.seek(exin.offset() + dsize);
					}
				}
				items.insert(std::make_pair(h.name, h));
			}
		}
	private:
	};

	/// zip reader
	reader::reader(io::read_map& ar)
		: m_imp(new zip_package_imp(ar))
	{}
	reader::reader(){}
	reader::~reader(){}
	reader::reader(reader&& rhs)
		: m_imp(std::move(rhs.m_imp))
	{}
	reader& reader::operator=(reader&& rhs){
		swap(rhs);
		return *this;
	}
	void reader::swap(reader& rhs){
		m_imp.swap(rhs.m_imp);
	}

	bool reader::valid() const{
		return m_imp;
	}
	reader::operator bool() const{
		return valid();
	}

	bool reader::exists(const file_path& name) const{
		AIO_PRE_CONDITION(valid());
		return m_imp->items.count(name) != 0;
	}
	const file_header* reader::get_file(const file_path& name) const{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(exists(name));
		auto pos = m_imp->items.find(name);
		return pos == m_imp->items.end() ? 0 : &pos->second;
	}

	template<typename F, typename L>
	struct select_second_{
		typedef L value_type;
		typedef L* pointer;
		typedef L& reference;

			reference operator()(std::pair<const F,L>& p) const{
				return p.second;
			}
	};
	typedef select_iterator<std::map<file_path, file_header, path_less>::iterator,
			select_second_<file_path, file_header> > wraped_iterator;
	range<reader::iterator> reader::items() const{
		AIO_PRE_CONDITION(valid());
		auto first = wraped_iterator(m_imp->items.begin());
		auto last = wraped_iterator(m_imp->items.end());
		return range<iterator>(first, last);
	}
	range<reader::iterator> reader::items(const file_path& dir) const{
		AIO_PRE_CONDITION(valid());
		auto res = m_imp->items.equal_range(dir);
		return range<iterator>(wraped_iterator(res.first), wraped_iterator(res.second));
	}

	class zip_package_writer_imp : public zip_package_imp
	{
		public:
			zip_package_writer_imp(iref<io::read_map, io::write_map> ar)
				: zip_package_imp(ar.get<io::read_map>())
				  , archive(ar)
		{
			file_header* fh = 0;
			for (auto &i : items){
				if (end_of_last < i.second.relative_offset_){
					fh = &i.second;
					end_of_last = i.second.relative_offset_;
				}
			}
			if (fh){
				auto view = package->view_rd(ext_heap::handle(fh->relative_offset_, fh->relative_offset_ + K_fix_part_of_local_header));
				auto address = view.get<io::read_view>().address();
				io::buffer_in  bin(address);
				auto sin = io::exchange::as_source(bin);
				sin & io::skip_n<K_fix_part_of_local_header - 4>();
				uint16_t name_len = io::load<uint16_t>(sin);
				uint16_t extra_len = io::load<uint16_t>(sin);
				end_of_last += K_fix_part_of_local_header + name_len + extra_len + fh->compressed_size;
			}
		}

			iref<io::read_map, io::write_map> archive;
			long_size_t end_of_last = 0;
			bool dirty = false;
	};

	/// zip reader_writer
	reader_writer::reader_writer(){}
	reader_writer::~reader_writer(){ sync();}
	reader_writer::reader_writer(const iref<io::read_map, io::write_map>& ar)
		: m_imp(new zip_package_writer_imp(ar))
	{
	}
	reader_writer::reader_writer(reader_writer&& rhs)
		: m_imp(std::move(rhs.m_imp))
	{}
	reader_writer& reader_writer::operator=(reader_writer&& rhs){
		swap(rhs);
		return *this;
	}
	void reader_writer::swap(reader_writer& rhs){
		m_imp.swap(rhs.m_imp);
	}

	bool reader_writer::valid() const{
		return m_imp;
	}
	reader_writer::operator bool() const{
		return valid();
	}

	bool reader_writer::exists(const file_path& name) const{
		AIO_PRE_CONDITION(valid());
		return m_imp->items.count(name) != 0;
	}
	const file_header* reader_writer::get_file(const file_path& name) const{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(exists(name));
		auto pos = m_imp->items.find(name);
		return pos == m_imp->items.end() ? 0 : &pos->second;
	}

	range<reader_writer::iterator> reader_writer::items() const{
		AIO_PRE_CONDITION(valid());
		auto first = wraped_iterator(m_imp->items.begin());
		auto last = wraped_iterator(m_imp->items.end());
		return range<iterator>(first, last);
	}
	range<reader_writer::iterator> reader_writer::items(const file_path& dir) const{
		AIO_PRE_CONDITION(valid());
		auto res = m_imp->items.equal_range(dir);
		return range<iterator>(wraped_iterator(res.first), wraped_iterator(res.second));
	}
	template<typename IoType>
	const file_header* append_(IoType& ar, const file_header& h_, file_type type, zip_package_writer_imp * m_imp){
		file_header h = h_;
		AIO_PRE_CONDITION(m_imp);
		AIO_PRE_CONDITION(h.method == cm_deflate || h.method == cm_store);
		AIO_PRE_CONDITION(h.name.str().size() < uint16_t(-1) );
		AIO_PRE_CONDITION(h.comments.size() < uint16_t(-1) );
		if (m_imp->items.count(h.name) > 0) return 0;

		auto header_size = K_fix_part_of_local_header + h.name.str().size() + K_sizeof_zip64_extra_field;
		auto dest_map = io::decorate<io::tail_archive
			, io::tail_read_map_p
			, io::tail_write_map_p
			>(m_imp->archive, m_imp->end_of_last + header_size);
		m_imp->dirty = true;
		if (h.method == cm_deflate){
			if (type == ft_raw){
				deflate_writer d_writer(dest_map);
				iref<io::writer> dest(d_writer);
				io::copy_data(ar, dest.get<io::writer>());
				d_writer.finish();

				h.uncompressed_size = d_writer.uncompressed_size();
				h.compressed_size = d_writer.size();
				h.crc32 = d_writer.crc32();
			}
			else {// type == ft_defalted
				io::copy_data(ar, dest_map);
			}
		}
		else {	// == cm_store
			auto size = io::copy_data(ar, dest_map);
			h.uncompressed_size = size;
			h.compressed_size = size;
			h.crc32 = crc32(dest_map);
		}
		setDateTime(h);
		h.relative_offset_ = m_imp->end_of_last;
		bool zip64_enabled = (h.uncompressed_size >= uint32_t(-1)
				|| h.compressed_size >= uint32_t(-1)
				|| h.relative_offset_ >= uint32_t(-1)
		   );
		{	// fill extra data for zip64
			io::buffer_out bout(h.extra);
			auto saver = io::exchange::as_sink(bout);
			save(saver, uint16_t(K_z64_extra_id)); //
			save(saver, uint16_t(28));		//sizeof extra data
			save(saver, uint64_t(h.uncompressed_size));
			save(saver, uint64_t(h.compressed_size));
			save(saver, uint64_t(h.relative_offset_));
			save(saver, uint32_t(0));		//Disk Start Number
		}
		auto hview = m_imp->archive.get<io::write_map>().view_wr(ext_heap::handle(m_imp->end_of_last
					, m_imp->end_of_last + header_size));
		io::fixed_buffer_io bout(hview.get<io::write_view>().address());
		auto saver = io::exchange::as_sink(bout);
		saver & K_sig_local_eader;
		saver & h.reader_version
			& h.flags
			& h.method
			& h.modified_time
			& h.modified_date
			& h.crc32;
		if (zip64_enabled)
			saver & uint32_t(-1) & uint32_t(-1);
		else
			saver & uint32_t(h.compressed_size) & uint32_t(h.uncompressed_size);
		saver & uint16_t(h.name.str().size()) & uint16_t(h.extra.size());
		bout.write(make_range(reinterpret_cast<const byte*>(h.name.str().begin())
					, reinterpret_cast<const byte*>(h.name.str().end())));
		bout.write(to_range(h.extra));

		h.local_header_size_ = header_size;
		auto pos = m_imp->items.insert(std::make_pair(h.name, h));
		m_imp->end_of_last += header_size + h.compressed_size;

		return &pos.first->second;
	}
	const file_header* reader_writer::append(io::read_map& ar, const file_path& name, compress_method method /* = cm_deflate */){
		file_header h;
		h.method = method;
		h.name = name;
		h.flags = 0x800;
		h.external_attrs = 0;
		return append_(ar, h, ft_raw, m_imp.get());
	}
	const file_header* reader_writer::append(io::reader& ar, const file_path& name, compress_method method /* = cm_deflate */){
		file_header h;
		h.method = method;
		h.name = name;
		h.flags = 0x800;
		h.external_attrs = 0;
		return append_(ar, h, ft_raw, m_imp.get());
	}

	const file_header* reader_writer::append(io::reader& ar, const file_header& h_, file_type type){
		return append_(ar, h_, type, m_imp.get());
	}
	const file_header* reader_writer::append(io::read_map& ar, const file_header& h, file_type type /* = ft_raw */){
		return append_(ar, h, type, m_imp.get());
	}
	void reader_writer::sync(){
		AIO_PRE_CONDITION(valid());

		if (!m_imp->dirty)
			return;

		bool zip64_enabled = false;
		long_size_t cd_size = 0;
		for (auto & i : m_imp->items){
			file_header& h = i.second;
			cd_size += K_fix_part_of_central_header 
				+ h.name.str().size() 
				+ h.comments.size();
			auto need_zip64 = h.compressed_size >= uint32_t(-1)
					|| h.uncompressed_size >= uint32_t(-1)
					|| h.relative_offset_ >= uint32_t(-1);
			zip64_enabled = zip64_enabled || need_zip64;
			if (need_zip64)
				cd_size += h.extra.size();
		}
		if (m_imp->items.size() >= uint16_t(-1) 
				|| cd_size >= uint32_t(-1)
				|| m_imp->end_of_last >= uint32_t(-1))
			zip64_enabled = true;
		uint16_t num_entries = m_imp->items.size() >= uint16_t(-1) ? uint16_t(-1) :  uint16_t(m_imp->items.size());

		auto cd_and_end_size = cd_size +  K_sizeof_cd_end;
		if (zip64_enabled)
			cd_and_end_size += K_sizeof_zip64_end_cd + K_sizeof_zip64_locator;
		auto view = m_imp->archive.get<io::write_map>().view_wr(ext_heap::handle(m_imp->end_of_last, m_imp->end_of_last + cd_and_end_size));
		auto address = view.get<io::write_view>().address();
		io::fixed_buffer_io bout(address);
		auto saver = io::exchange::as_sink(bout);
		for (auto & i : m_imp->items){
			file_header& h = i.second;
			auto need_zip64 = h.compressed_size >= uint32_t(-1)
					|| h.uncompressed_size >= uint32_t(-1)
					|| h.relative_offset_ >= uint32_t(-1);

			saver & K_sig_central_file_header
				& h.creator_version
				& h.reader_version
				& h.flags
				& h.method
				& h.modified_time

				& h.modified_date
				& h.crc32;
			if (need_zip64)
				saver & uint32_t(-1) & uint32_t(-1);
			else
				saver & uint32_t(h.compressed_size) & uint32_t(h.uncompressed_size);

			saver & uint16_t(h.name.str().size())
				& uint16_t(need_zip64 ? h.extra.size() : 0)
				& uint16_t(h.comments.size())
				& uint16_t(0)	//disk number start
				& uint16_t(0)	//internal file attributes
				& h.external_attrs;
			if (need_zip64)
				saver & uint32_t(-1);
			else
				saver & uint32_t(h.relative_offset_);
			bout.write(make_range(reinterpret_cast<const byte*>(h.name.str().begin())
						, reinterpret_cast<const byte*>(h.name.str().end())));
			if (need_zip64)
				bout.write(to_range(h.extra));
			bout.write(make_range(reinterpret_cast<const byte*>(h.comments.begin())
						, reinterpret_cast<const byte*>(h.comments.end())));
		}
		if (zip64_enabled){
			saver & K_sig_zip64_end_cd
				& uint64_t(cd_size)
				& uint16_t(20) //version made by
				& uint16_t(20) //version needed to extract
				& uint32_t(0) //number of this disk
				& uint32_t(0) //number of the disk with the start of the central directory
				& uint64_t(m_imp->items.size()) //total number of entries in the central directory on this disk
				& uint64_t(m_imp->items.size()) //total number of entries in the central directory
				& uint64_t(cd_size) //size of the central directory
				& uint64_t(m_imp->end_of_last); //offset of start of central directory with respect to the starting disk number

			saver & K_sig_zip64_end_of_cd_locator
				& uint32_t(0)	//number of the disk with the start of the zip64 end of central directory
				& uint64_t(cd_size + m_imp->end_of_last)	//relative offset of the zip64 end of central directory record
				& uint32_t(1);		//total number of disks

		}


		saver & K_sig_end_central_dir_signature
			& uint16_t(0)	//number of this disk
			& uint16_t(0)	//number of the disk with the start of the central directory
			& num_entries 	//total number of entries in the central directory on this disk
			& num_entries 	//total number of entries in the central directory
			& (cd_size >= uint32_t(-1)? uint32_t(-1) : uint32_t(cd_size))	//size of the central directory
			& (m_imp->end_of_last >= uint32_t(-1)? uint32_t(-1) : uint32_t(m_imp->end_of_last))	//offset of start of central directory with respect to the starting disk number
			& uint16_t(0);	//.ZIP file comment length


		m_imp->archive.get<io::write_map>().sync();
		m_imp->dirty = false;

	}

	iauto<io::read_map> open_raw(const file_header& fh){
		AIO_PRE_CONDITION(fh.package);

		if (fh.local_header_size_ == uint16_t(-1)){
			auto view = fh.package->view_rd(ext_heap::handle(fh.relative_offset_, fh.relative_offset_ + K_fix_part_of_local_header));
			auto address = view.get<io::read_view>().address();
			io::buffer_in  bin(address);
			auto sin = io::exchange::as_source(bin);
			sin & io::skip_n<K_fix_part_of_local_header - 4>();
			uint16_t name_len = io::load<uint16_t>(sin);
			uint16_t extra_len = io::load<uint16_t>(sin);
			const_cast<file_header&>(fh).local_header_size_ = K_fix_part_of_local_header + name_len + extra_len;
		}

		auto adaptor = io::decorate<io::sub_archive, io::sub_read_map_p>(fh.package, 
				fh.relative_offset_ + fh.local_header_size_, fh.relative_offset_ + fh.local_header_size_ + fh.compressed_size);
		return iauto<io::read_map>(std::move(adaptor));
	}
}}

