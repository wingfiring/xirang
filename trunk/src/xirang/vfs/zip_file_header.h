#ifndef AIO_XIRANG_ZIP_FILE_HEADER_H
#define AIO_XIRANG_ZIP_FILE_HEADER_H
#include <aio/xirang/vfs/zip.h>
#include <aio/common/archive/mem_archive.h>
#include <tuple>

#include <stdint.h>
namespace xirang{ namespace fs{ 

	struct file_header
	{
		/// fields from zip file.
		uint16_t gp_flag;
		uint16_t compression_method;	//store, defate etc
        uint16_t mod_time, mod_date;
		uint32_t in_crc32, compressed_size, uncompressed_size;
		uint32_t relative_offset_local_header;
		uint32_t relative_offset_data;
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
			relative_offset_data(-1),

			type(aiofs::st_invalid),
            dirty(true),
			persist(true)
		{
		}

	};
#if 0
	struct file_header;

	archive_ptr inflate_header(file_header* fh);
	struct file_header
	{
		uint16_t gp_flag;
		uint16_t compression_method;	//store, defate etc
        uint16_t mod_time, mod_date;
		uint32_t in_crc32, compressed_size, uncompressed_size;
		uint32_t relative_offset_local_header;
		uint32_t relative_offset_data;
		string name,comment;


		file_state type;
		bool cached;
        bool dirty;
		IVfs* cache_fs;

		file_header() :
			gp_flag(0),
			compression_method(0),
            mod_time(0), mod_date(0), 
			in_crc32(0), compressed_size(0), uncompressed_size(0),
			relative_offset_local_header(-1),
			relative_offset_data(-1),

			type(aiofs::st_invalid),
            cached(false),
            dirty(true),
			cache_fs(0),
		{
		}

		void** cache(unsigned long long mask,
			void** base, aio::unique_ptr<void>& owner, const string& path, int flag,
			read_map* rmap, write_map* wmap){

			AIO_PRE_CONDITION(!name.empty());
            if (!cached && this->relative_offset_data != uint32_t(-1))
            {
                aio::archive::archive_ptr ret = inflate_header(this, zipfile);
                cached = true;
                return ret;
            }
            cached = true;

            dirty = dirty || type == will_edit;

            if (this->relative_offset_data == uint32_t(-1))
            {
                return recursive_create(*cache_fs, name
                    , aio::archive::mt_random | aio::archive::mt_read | aio::archive::mt_write
                    , aio::archive::of_create_or_open);
            }

            return cache_fs->create(name
                , aio::archive::mt_random | aio::archive::mt_read | aio::archive::mt_write
                , aio::archive::of_open);
		}
	};


	int load_cd(iarchive& file, aio::buffer<byte>& buf);
	void load_header(aio::archive::buffer_in& ar, file_header& h);
	void copy_entry(file_header& h, aio::archive::writer& wr, aio::archive::random& rng);

	void write_cd_entry(file_header& h, aio::archive::writer& wr);
	void write_cd_end(aio::archive::writer& wr, size_t num_entries, aio::long_size_t size_central_dir, aio::long_size_t offset_central_dir);

#endif
	/// \return data view & start pos in the view
	/// \return view of cd & offset without the central dir signature
	std::tuple<aio::iauto<aio::io::read_view>, uint32_t> load_cd(aio::io::read_map& file);
	file_header load_header(aio::iref<aio::io::reader, aio::io::random> iar);
}}

#endif // end AIO_XIRANG_ZIP_FILE_HEADER_H

