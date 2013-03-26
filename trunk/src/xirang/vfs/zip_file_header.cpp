#include "zip_file_header.h"
#include <boost/numeric/conversion/cast.hpp>
#include <zlib.h>
#include <boost/tokenizer.hpp>

#include <aio/common/to_string.h>

#include <iostream>
#include <ctime>

namespace xirang{ namespace fs{ 
	const uint32_t sig_end_central_dir_signature = 0x06054b50;
	const uint32_t sig_central_file_header = 0x02014b50;
	const aio::long_size_t max_head_size = 64 * 1024;	//64k
	const uint32_t sig_local_file_header = 0x04034b50;

	using boost::numeric_cast;
    using aio::long_size_t;
#if 0
    void setDateTime(file_header& h)
    {
        AIO_PRE_CONDITION(h.mod_time == uint16_t(-1));
        std::time_t rawtime = time(0);
        struct std::tm* ptm = localtime(&rawtime);
        h.mod_date = ((ptm->tm_year - 80) << 9)  + ((ptm->tm_mon + 1) << 5) + ptm->tm_mday;
        h.mod_time = (ptm->tm_hour << 11) + (ptm->tm_min << 5) + (ptm->tm_sec >> 1);
    }

    struct zip_inflater
    {
        z_stream zstream;
        zip_inflater()
        {
            zstream = z_stream();
            if (inflateInit2(&zstream, -MAX_WBITS) != Z_OK)
                AIO_THROW(archive_runtime_error);
        }
        void do_inflate(aio::long_size_t datasize, aio::archive::reader& rd, aio::archive::writer& wr)
        {
            aio::buffer<byte> bin, bout;
            bin.resize(64 * 1024);
            bout.resize(64 * 1024);

            int res = Z_OK;
            while (datasize > 0)
            { 
                aio::long_size_t rd_size = bin.size() <  datasize ? bin.size() : datasize;
                bin.resize(numeric_cast<size_t>(rd_size));

                aio::buffer<byte>::iterator rdin = rd.read(to_range(bin));
                if (rdin == bin.begin())
                {
                    //TODO: yield or sleep
                    continue;
                }

                zstream.avail_in = uInt(rdin - bin.begin());
                zstream.next_in = (Bytef*)&bin[0];
                do {
                    zstream.avail_out = uInt(bout.size());
                    zstream.next_out = (Bytef*)&bout[0];
                    res = ::inflate(&zstream, Z_NO_FLUSH);
                    switch (res)
                    {
                    case Z_NEED_DICT:
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        AIO_THROW(archive_runtime_error)("inflate bad data");
                    }

                    block_write(wr, aio::make_range(bout.begin(), bout.begin() + (bout.size() - zstream.avail_out)));
                } while(zstream.avail_out == 0);

                datasize -= rdin - bin.begin();
            }
            if (res != Z_STREAM_END)
            {
                AIO_THROW(archive_runtime_error)("bad data, need more");
            }
        }
        ~zip_inflater()
        {
            inflateEnd(&zstream);
        }
    };

	archive_ptr inflate_header(file_header* fh, read_map& rmap)
	{
		AIO_PRE_CONDITION(fh);
		AIO_PRE_CONDITION(fh->cache_fs);
        AIO_PRE_CONDITION(fh->relative_offset_data != uint32_t(-1));


        aio::archive::archive_ptr par = recursive_create(*fh->cache_fs, fh->name
            , aio::archive::mt_random | aio::archive::mt_read | aio::archive::mt_write
			, aio::archive::of_create_or_open);

		if (!par)
			AIO_THROW(vfs_runtime_error)("failed to create cached file.");

        aio::archive::writer* wr = par->query_writer();
		wr->truncate(0);


		rdrng->seek(fh->relative_offset_data);

		if (fh->compression_method == 0) //store
		{
			long_size_t copied_size = copy_data(*rd, *wr, fh->compressed_size);
			if (copied_size != fh->compressed_size)
				AIO_THROW(archive_runtime_error)("data lost.");
		}
		else if(fh->compressed_size > 0)
		{
            zip_inflater().do_inflate(fh->compressed_size, *rd, *wr);
		}
        return std::move(par);
	}

	std::tuple<aio::iauto<aio::io::read_view>, long_size_t> load_cd(aio::io::read_map& file, aio::buffer<byte>& buf)
	{
		// seek to load central dir end
		// read the end 64k bytes. central dir end is never larger than 64k
		long_size_t size = file.size();
		long_size_t off = size - std::min(size, max_head_size);
		auto view = file.view_rd(aio::ext_heap::handle(off, size));
		auto address = view.address();
		if (address.size() == 0)
			AIO_THROW(bad_end_central_dir_signature)("end central dir signature is not found");

		// reverse find the signature
		typedef std::reverse_iterator<const byte*> iterator;
		iterator rbeg(address.end()), rend(address.begin());
		const byte* sig_beg = reinterpret_cast<const byte*>(&sig_end_central_dir_signature);
		const byte* sig_end = sig_beg + sizeof(sig_end_central_dir_signature);

		iterator rsbeg(sig_end), rsend(sig_beg);
		iterator pos = search(rbeg, rend, rsbeg, rsend);
		if (pos == rend) //not found
			AIO_THROW(bad_end_central_dir_signature)("end central dir signature is not found");

		return std::tuple<aio::iauto<aio::io::read_view>, long_size_t>(std::move(view), pos.base() - address.begin());

		//load offset info
		aio::archive::buffer_in  mrd(buf);
		mrd.seek(pos.base() - buf.begin());

		uint16_t number_entries = load<uint16_t>(mrd);
		uint32_t size_central_dir = load<uint32_t>(mrd);
		uint32_t offset_central_dir = load<uint32_t>(mrd);

		buf.resize(size_central_dir);
		rng->seek(offset_central_dir);
		aio::buffer<byte>::iterator end_cd_itr = block_read(*rd, to_range(buf));
		if (end_cd_itr != buf.end())	// not read enough content
			AIO_THROW(bad_central_dir_offset_or_size);

		return number_entries;
	}

	void load_header(aio::archive::buffer_in& ar, file_header& h)
	{
		//Begin read entry
		uint16_t skip16;

		uint32_t sig; ar & sig;
		if(sig != sig_central_file_header)
			AIO_THROW(bad_central_file_header_signature);

		ar & skip16 			// version made by
			& skip16			// version need to extract
			& h.gp_flag 		// general purpose bit flag
			& h.compression_method // compression method
			& h.mod_time		// last mod file time
            & h.mod_date		// last mod file date
			& h.in_crc32 			// crc-32
			& h.compressed_size // compressed size
			& h.uncompressed_size; // uncompressed size
		uint16_t fsize; 		// file name length
		uint16_t efsize; 		// extra field length
		uint16_t fcsize; 		// file comment length
		uint32_t ext_attributes;
		const uint32_t dir_mask = 0x10;//both dos and unix are same
		ar & fsize & efsize & fcsize
			& skip16 			// disk number start
			& skip16 			// internal file attribute
			& ext_attributes;	// external file attribute

		h.type = ext_attributes & dir_mask 
			? aiofs::st_dir : aiofs::st_regular; // external file attribute
		ar & h.relative_offset_local_header; // relative offset of local header
		//h.relative_offset_data = h.relative_offset_local_header + msize_local_file_header + fsize + efsize;
		if (fsize > ar.size() - ar.offset())
			AIO_THROW(bad_no_enough_head_content);

		aio::string_builder tsz(numeric_cast<size_t>(fsize), char('\0'));
        aio::range<aio::buffer<byte>::iterator> mtsz = string_to_range(tsz);
		ar.read(mtsz);
		h.name = tsz;

		ar.seek(ar.offset() + efsize + fcsize);
		if (ar.offset() > ar.size())
			AIO_THROW(bad_no_enough_head_content);

		//End read zip entry
	}

    struct zip_defalter
    {
        z_stream zstream;
        zip_defalter()
        {
            zstream = z_stream();
            if (deflateInit2(&zstream, Z_BEST_SPEED, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK)
				AIO_THROW(archive_runtime_error);
        }
        uint32_t do_deflate(aio::archive::reader& rd, aio::archive::writer& wr)
        {
            
			uint32_t in_crc32 = crc32(0L, Z_NULL, 0);

			aio::buffer<byte> bin, bout;
			bin.resize(64 * 1024);
			bout.resize(64 * 1024);

			zstream.next_out = (Bytef*)bout.begin();
			zstream.avail_out = uInt(bout.size());
            bool need_output = rd.readable();
			while (rd.readable() || need_output)
			{
				//flush output buffer
				if (zstream.next_out != (Bytef*)bout.begin())	//avaliable output
				{
					//h.in_crc32 = crc32(h.in_crc32, bout.begin(), zstream.next_out - bout.begin());
                    aio::archive::writer::const_iterator witr = block_write(wr, aio::make_range(bout.begin(), (byte*)zstream.next_out));
					if (witr != (byte*)zstream.next_out)
						AIO_THROW(archive_io_fatal_error);

					zstream.next_out = (Bytef*)bout.begin();
					zstream.avail_out = uInt(bout.size());
				}

				if (zstream.avail_in == 0 && rd.readable())
				{
                    aio::buffer<byte>::iterator rdin = rd.read(to_range(bin));
					if (rdin == bin.begin()) //read nothing, continue
						continue;

					zstream.next_in = (Bytef*)bin.begin();
					zstream.avail_in = uInt(rdin - bin.begin());
					in_crc32 = crc32(in_crc32, zstream.next_in, zstream.avail_in);
				}

				if (rd.readable())
				{
					if (deflate(&zstream, Z_NO_FLUSH) != Z_OK)
						AIO_THROW(archive_runtime_error);
				}
				else
				{
					int ret = deflate(&zstream, Z_FINISH);

					if (ret == Z_STREAM_END)
                    {
                        aio::archive::writer::const_iterator witr = block_write(wr, aio::make_range(bout.begin(), (byte*)zstream.next_out));                    
                        if (witr != (byte*)zstream.next_out)
                            AIO_THROW(archive_io_fatal_error);                    
						break;
                    }
					else if (ret == Z_OK)
                        continue;
                    else
						AIO_THROW(archive_runtime_error);

				}
			}
            return in_crc32;
        }
        ~zip_defalter()
        {
            deflateEnd(&zstream);
        }
    };
	void copy_entry(file_header& h, aio::io::read_map& rd, aio::io::write_map& wr, aio::archive::random& rng)
	{
        AIO_PRE_CONDITION (h.type == aiofs::st_regular);

		long_size_t local_header_off = rng.offset();
        if (h.mod_time == uint16_t(-1))
            setDateTime(h);

		wr & sig_local_file_header 	//4B signature
			& uint16_t(20)			//2B version need to extract
			& h.gp_flag 			//2B general propose bit flag
			& h.compression_method	//2B
            & h.mod_time			//2B modified time TODO: imp
            & h.mod_date			//2B modified date
			& h.in_crc32				//4B will be modified in update_local_header
			& h.compressed_size		//4B same as above
			& h.uncompressed_size	//4B same as above
			& uint16_t(h.name.size())	//2B file name length
			& uint16_t(0)			//2B extra fileld length
			;
        aio::range<aio::buffer<byte>::const_iterator> cname = string_to_c_range(h.name);
        aio::archive::writer::const_iterator name_pos = block_write(wr, cname);
		if (name_pos != cname.end())
			AIO_THROW(archive_io_fatal_error);
		long_size_t local_data_off = rng.offset();

        if (!h.cached || !h.dirty)
		{
			h.zip_archive->query_random()->seek(h.relative_offset_data(rd));
			long_size_t copied_size = copy_data(*h.zip_archive->query_reader(), wr, h.compressed_size);
			if (copied_size != h.compressed_size)
				AIO_THROW(archive_io_fatal_error);
		}
        else 
		{
            zip_defalter defalter;

            aio::archive::archive_ptr hcache = h.cache(no_edit);
			aio::archive::reader* rd = hcache->query_reader();
            h.in_crc32 = defalter.do_deflate(*rd, wr);
            h.compressed_size = defalter.zstream.total_out;
			h.uncompressed_size = defalter.zstream.total_in;

			long_size_t pos = rng.offset();
			rng.seek(local_header_off + 14); //offset of in_crc32

			wr & h.in_crc32 & h.compressed_size & h.uncompressed_size;
			rng.seek(pos);
		}

		h.relative_offset_local_header = numeric_cast<uint32_t>(local_header_off);
		h.relative_offset_data = numeric_cast<uint32_t>(local_data_off);
	}

	void write_cd_entry(file_header& h, aio::archive::writer& wr)
	{
        AIO_PRE_CONDITION (h.type == aiofs::st_regular);

        if (h.mod_time == uint16_t(-1))
            setDateTime(h);

		wr 
			& sig_central_file_header
			& uint16_t(20)				//2B version made by
			& uint16_t(20)				//2B version need to extract
			& h.gp_flag					//2B general purpose bit flag
			& h.compression_method
            & h.mod_time			//2B modified time TODO: imp
            & h.mod_date			//2B modified date
			& h.in_crc32				//4B will be modified in update_local_header
			& h.compressed_size		//4B same as above
			& h.uncompressed_size	//4B same as above
			& uint16_t(h.name.size())	//2B file name length
			& uint16_t(0)			//2B extra fileld length
			& uint16_t(0)			//2B file comment length
			& uint16_t(0)			//2B disk number start
			& uint16_t(0)			//2B internal file attributes
			& (h.type == aiofs::st_regular ? uint32_t(0) : uint32_t(0x10)) //4B external file attribute
			& h.relative_offset_local_header	//4B
			;
		aio::range<aio::buffer<byte>::const_iterator> cname = string_to_c_range(h.name);
        aio::archive::writer::const_iterator name_pos = block_write(wr, cname);
		if (name_pos != cname.end())
			AIO_THROW(archive_io_fatal_error);

	}
	void write_cd_end(aio::archive::writer& wr, size_t num_entries, long_size_t size_central_dir, long_size_t offset_central_dir)
	{
		wr 
			& sig_end_central_dir_signature
			& uint16_t(0)				//2B number of this disk
			& uint16_t(0)				//2B number of the central start disk
			& uint16_t(num_entries)		//2B total number of entries on this disk
			& uint16_t(num_entries)		//2B total number of entries
			& uint32_t(size_central_dir) //4B size of the CD
			& uint32_t(offset_central_dir) 	//4B offset of CD
			& uint16_t(0)					//2B ZIP file comment length
			;
	}
#endif
	std::tuple<aio::iauto<aio::io::read_view>, uint32_t> load_cd(aio::io::read_map& file)
	{
		AIO_PRE_CONDITION(file.size() > 0);

		typedef std::tuple<aio::iauto<aio::io::read_view>, aio::long_size_t> return_type;
		// seek to load central dir end
		// read the end 64k bytes. central dir end is never larger than 64k
		long_size_t size = file.size();
		long_size_t off = size - std::min(size, max_head_size);
		auto view = file.view_rd(aio::ext_heap::handle(off, size));
		auto address = view.get<aio::io::read_view>().address();
		if (address.size() == 0)
			AIO_THROW(archive_io_fatal_error)("failed to read central dir");

		// reverse find the signature
		typedef std::reverse_iterator<const byte*> iterator;
		iterator rbeg(address.end()), rend(address.begin());
		const byte* sig_beg = reinterpret_cast<const byte*>(&sig_end_central_dir_signature);
		const byte* sig_end = sig_beg + sizeof(sig_end_central_dir_signature);
		iterator rsbeg(sig_end), rsend(sig_beg);
		iterator pos = search(rbeg, rend, rsbeg, rsend);

		if (pos == rend) //if not found
			AIO_THROW(bad_end_central_dir_signature)("end central dir signature is not found");

		// return the view & offset without the central dir signature
		return return_type(std::move(view), pos.base() - address.begin());

		aio::io::buffer_in mrd(make_range(pos.base(), address.end()));
		//skip 1.number of disk 2. number of central start disk 3. number of entries on this disk
		mrd.seek(mrd.offset() + sizeof(uint16_t) * 3); 
		uint16_t number_entries = aio::sio::load<uint16_t>(mrd);
		uint32_t size_central_dir = aio::sio::load<uint32_t>(mrd);
		uint32_t offset_central_dir = aio::sio::load<uint32_t>(mrd);

		aio::ext_heap::handle hcd(offset_central_dir, offset_central_dir + size_central_dir);

		return return_type(file.view_rd(hcd), number_entries);
	}

	file_header load_header(aio::iref<aio::io::reader, aio::io::random> iar)
	{
		aio::io::reader& ar = iar.get<aio::io::reader>();
		aio::io::random& rng = iar.get<aio::io::random>();
		//Begin read entry

		uint32_t sig = aio::sio::load<uint32_t>(ar);
		if(sig != sig_central_file_header)
			AIO_THROW(bad_central_file_header_signature);

		using namespace aio::sio;
		uint16_t skip16;
		file_header h= {};
		h.dirty = false;
		h.persist = true;

		ar & skip16 			// version made by
			& skip16			// version need to extract
			& h.gp_flag 		// general purpose bit flag
			& h.compression_method // compression method
			& h.mod_time		// last mod file time
            & h.mod_date		// last mod file date
			& h.in_crc32 			// crc-32
			& h.compressed_size // compressed size
			& h.uncompressed_size; // uncompressed size
		uint16_t fsize = load<uint16_t>(ar); 		// file name length
		uint16_t efsize = load<uint16_t>(ar); 		// extra field length
		uint16_t fcsize = load<uint16_t>(ar); 		// file comment length
		ar & skip16 & skip16;	//skip disk number start & internal file attribute
		uint32_t ext_attributes = load<uint32_t>(ar);
		const uint32_t dir_mask = 0x10;//both dos and unix are same

		h.type = ext_attributes & dir_mask 
			? aiofs::st_dir : aiofs::st_regular; // external file attribute

		ar & h.relative_offset_local_header; // relative offset of local header
		//h.relative_offset_data = h.relative_offset_local_header + msize_local_file_header + fsize + efsize;
		if (fsize > rng.size() - rng.offset())
			AIO_THROW(bad_no_enough_head_content);

		aio::string_builder tsz(numeric_cast<size_t>(fsize), char('\0'));
		ar.read(string_to_range(tsz));
		h.name = aio::fs::to_aio_path(tsz.str());

		rng.seek(rng.offset() + efsize + fcsize);	// seek to begin of next cd item
		if (rng.offset() > rng.size())
			AIO_THROW(bad_no_enough_head_content);

		//End read zip entry
		return h;
	}

}}

