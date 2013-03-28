#ifndef AIO_XIRANG_ZIP_FILE_HEADER_H
#define AIO_XIRANG_ZIP_FILE_HEADER_H
#include <aio/xirang/vfs/zip.h>
#include <aio/common/archive/mem_archive.h>
#include <tuple>

#include <stdint.h>
namespace xirang{ namespace fs{ 
	const uint32_t msize_local_file_header = 30;

	struct file_header
	{
		/// fields from zip file.
		uint16_t gp_flag;
		uint16_t compression_method;	//store, defate etc
        uint16_t mod_time, mod_date;
		uint32_t in_crc32, compressed_size, uncompressed_size;
		uint32_t relative_offset_local_header;
		mutable uint32_t relative_offset_data_;
		string name, comment;

		file_state type;
		string cached_path;
        bool dirty;
        bool persist;

		file_header() :
			gp_flag(0),
			compression_method(0),
            mod_time(0), mod_date(0), 
			in_crc32(0), compressed_size(0), uncompressed_size(0),
			relative_offset_local_header(-1),
			relative_offset_data_(-1),

			type(aiofs::st_invalid),
            dirty(true),
			persist(true)
		{
		}
		bool cached() const { return !cached_path.empty();}
		uint32_t relative_offset_data(aio::io::read_map& rm) const{
			if (relative_offset_data_ == uint32_t(-1)){
				auto offset_of_name_len = relative_offset_local_header + msize_local_file_header - 4;
				auto view = rm.view_rd(ext_heap::handle(offset_of_name_len, offset_of_name_len + 4));
				aio::io::buffer_in bin (view.get<aio::io::read_view>().address());
				auto name_len = aio::sio::load<uint16_t>(bin);
				auto extra_len = aio::sio::load<uint16_t>(bin);
				relative_offset_data_ = relative_offset_local_header + msize_local_file_header + name_len + extra_len;
			}
			return relative_offset_data_;
		}
	};

	/// \return data view & start pos in the view
	/// \return view of cd & offset without the central dir signature
	std::tuple<aio::iauto<aio::io::read_view>, uint32_t> load_cd(aio::io::read_map& file);
	file_header load_header(const aio::iref<aio::io::reader, aio::io::random>& iar);

	void copy_entry(file_header& h, aio::io::write_map& dest);

	void write_cd_entry(file_header& h, aio::io::write_map& wr);
	void write_cd_end(aio::io::write_map& wr, size_t num_entries, aio::long_size_t size_central_dir, aio::long_size_t offset_central_dir);
}}

#endif // end AIO_XIRANG_ZIP_FILE_HEADER_H

