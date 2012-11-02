#ifndef AIO_XIRANG_ZIP_FILE_HEADER_H
#define AIO_XIRANG_ZIP_FILE_HEADER_H
#include <aio/xirang/vfs/zip.h>
#include <aio/common/archive/mem_archive.h>
#include <stdint.h>
namespace xirang{ namespace fs{ 
	struct file_header;

	archive_ptr inflate_header(file_header* fh);
    enum WillEdit
    {
        will_edit,
        no_edit
    };
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
		iarchive* zip_archive;
		bool cached;
        bool dirty;
		IVfs* cache_fs;
		int index;

		file_header() :
			gp_flag(0),
			compression_method(0),
            mod_time(0), mod_date(0), 
			in_crc32(0), compressed_size(0), uncompressed_size(0),
			relative_offset_local_header(-1),
			relative_offset_data(-1),

			type(aiofs::st_invalid),
			zip_archive(0),
            cached(false),
            dirty(true),
			cache_fs(0),
			index(-1)
		{
		}

		archive_ptr cache(WillEdit type) { 
			AIO_PRE_CONDITION(!name.empty());
            if (!cached && this->relative_offset_data != uint32_t(-1))
            {
                aio::archive::archive_ptr ret = inflate_header(this);
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


}}

#endif // end AIO_XIRANG_ZIP_FILE_HEADER_H

