#include <aio/common/deflate.h>
#include <zlib.h>

namespace aio{ namespace zip{
	namespace{
		const size_t BufferSize = 64 * 1024;
		const long_size_t ViewSize = 1024 * 1024;
	}

	uint32_t crc32_init(){
		return ::crc32(0L, Z_NULL, 0);
	}
	uint32_t crc32(io::reader& src, uint32_t crc /* = crc32_init()*/){
		aio::buffer<byte> bin;
		bin.resize(BufferSize);
		while (src.readable()){
			auto rest = src.read(to_range(bin));
			crc =  ::crc32(crc, (const Bytef*)bin.begin(), (uInt)(bin.size() - rest.size()));
		}
		return crc;
	}
	uint32_t crc32(io::read_map& src, uint32_t crc /* = crc32_init()*/){
		long_size_t in_pos = 0;
		long_size_t rest_in_size = src.size();

		while (rest_in_size > 0) { 
			auto rview = src.view_rd(ext_heap::handle(in_pos, in_pos + std::min(ViewSize, rest_in_size)));
			auto rng = rview.get<io::read_view>().address();
			crc = crc32(rng);
			in_pos += rng.size();
			rest_in_size -= rng.size();
		}
		return crc;
	}
	uint32_t crc32(range<const byte*>& src, uint32_t crc /* = crc32_init() */){
		return ::crc32(crc, (const Bytef*)src.begin(), (uInt)src.size());
	}

	namespace{

		static void* zip_malloc(void* h, uInt items, uInt size){
			heap* hp = (heap*)h;
			try{ return hp->malloc(items * size, 1, 0); } 
			catch(...){ return 0; }
		}

		static void zip_free(void* h, void* p){
			heap* hp = (heap*)h;
			hp->free(p, -1, 0);
		}


		struct zip_inflater
		{
			z_stream zstream;
			explicit zip_inflater(heap* h) 
				: zstream()
			{
				if (h){
					zstream.zalloc = zip_malloc;
					zstream.zfree = zip_free;
					zstream.opaque = h;
				}
			}
			int init() {
				return (inflateInit2(&zstream, -MAX_WBITS) == Z_OK)
					? ze_ok : ze_mem_error;
			}

			zip_result do_inflate(io::reader& rd, io::writer& wr, dict_type dict)
			{
				long_size_t in_bytes  = 0;
				long_size_t out_bytes = 0;

				aio::buffer<byte> bin, bout;
				bin.resize(BufferSize);
				bout.resize(BufferSize);

				while (rd.readable()) { 
					auto rest = rd.read(to_range(bin));
					auto input = aio::make_range(bin.begin(), rest.begin());
					if (input.empty()) 
						return zip_result{ze_stream_error, in_bytes, out_bytes};

					zstream.avail_in = uInt(input.size());
					zstream.next_in = (Bytef*)input.begin();
					do {
						zstream.avail_out = uInt(bout.size());
						zstream.next_out = (Bytef*)&bout[0];

						auto res = ::inflate(&zstream, Z_NO_FLUSH);
						in_bytes += input.size() - zstream.avail_in;
						out_bytes += bout.size() - zstream.avail_out;
						auto output = make_range(bout.begin(), bout.begin() + (bout.size() - zstream.avail_out));
						switch(res)
						{
							case Z_NEED_DICT:
								{
									if (dict.empty()) return zip_result{ze_need_dict, in_bytes, out_bytes };
									auto dic_err = inflateSetDictionary(&zstream, (const Bytef*)dict.begin(), dict.size());
									if (dic_err != Z_OK) return zip_result{ze_data_error, in_bytes, out_bytes};
								}
								break;
							case Z_DATA_ERROR:
								return zip_result{ze_data_error, in_bytes, out_bytes};
							case Z_MEM_ERROR:
								return zip_result{ze_mem_error, in_bytes, out_bytes};
							case Z_STREAM_END:
								block_write(wr, output);
								return zip_result{ze_ok, in_bytes, out_bytes};
						}

						block_write(wr, output);

					} while(zstream.avail_out == 0);
				}
				return {ze_unfinished_data, in_bytes, out_bytes};
			}

			zip_result do_inflate(io::read_map& rd, io::write_map& wr, dict_type dict)
			{
				typedef ext_heap::handle handle;

				long_size_t in_pos = 0;
				long_size_t out_pos = 0;
				long_size_t rest_in_size = rd.size();
				long_size_t rest_out_size = wr.size();

				iauto<io::read_view> rview;
				iauto<io::write_view> wview;
				range<const byte*> bin;
				range<byte*> bout;
				for(;;){

					if (rest_in_size > 0 && zstream.avail_in == 0){
						rview = rd.view_rd(handle(in_pos, in_pos + 
									std::min(ViewSize, (rest_in_size == 0 ? ViewSize : rest_in_size))));
						bin = rview.get<io::read_view>().address();
						zstream.avail_in = uInt(bin.size());
						zstream.next_in = (Bytef*)bin.begin();
						in_pos += bin.size();
						rest_in_size -= bin.size();
					}

					if (zstream.avail_out == 0){
						out_pos += bout.size();
						wview = wr.view_wr(handle(out_pos, out_pos + 
									std::min(ViewSize, (rest_out_size == 0 ? ViewSize : rest_out_size))));
						bout = wview.get<io::write_view>().address();
						zstream.avail_out = uInt(bout.size());
						zstream.next_out = (Bytef*)bout.begin();
					}

					auto res = ::inflate(&zstream, Z_NO_FLUSH);
					auto cur_out_pos = out_pos + (bout.size() - zstream.avail_out);
					switch(res)
					{
						case Z_NEED_DICT:
							{
								if (dict.empty()) return zip_result{ze_need_dict, in_pos, cur_out_pos};
								auto dic_err = inflateSetDictionary(&zstream, (Bytef*)dict.begin(), dict.size());
								if (dic_err != Z_OK) return zip_result{ze_data_error, in_pos, cur_out_pos};
							}
						case Z_OK:
							break;
						case Z_DATA_ERROR:
							return zip_result{ze_data_error, in_pos, cur_out_pos};
						case Z_MEM_ERROR:
							return zip_result{ze_mem_error, in_pos, cur_out_pos};
						case Z_STREAM_END:
							return zip_result{ze_ok, in_pos, cur_out_pos};
						default:
							return zip_result{ze_internal_error, in_pos, cur_out_pos};
					}
				}
			}
			~zip_inflater()
			{
				inflateEnd(&zstream);
			}
		};
	}

	zip_result inflate(io::reader& src, io::writer& dest, dict_type dict/* = dict_type() */, heap* h /* = 0 */)
	{
		zip_inflater inflater(h);

		auto err = inflater.init();
		if (err != ze_ok)
			return zip_result{err, 0, 0};

		return inflater.do_inflate(src, dest, dict);
	}

	zip_result inflate(io::read_map& src, io::write_map& dest, dict_type dict /* = dict_type() */ ,  heap* h /* = 0 */ )
	{
		zip_inflater inflater(h);

		auto err = inflater.init();
		if (err != ze_ok)
			return zip_result{err, 0, 0};

		return inflater.do_inflate(src, dest, dict);
	}


	namespace {
    struct zip_defalter
    {
        z_stream zstream;
		zip_defalter(heap* h)
			: zstream()
		{
			if (h){
				zstream.zalloc = zip_malloc;
				zstream.zfree = zip_free;
				zstream.opaque = h;
			}
		}

		int init(int level, int strategy_, dict_type dict) {
			switch (deflateInit2(&zstream, level, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, strategy_)){
				case Z_OK: return ze_ok;
				case Z_MEM_ERROR: return ze_mem_error;
				case Z_STREAM_END: return ze_stream_error;
				default:
								   return ze_internal_error;
			}
			switch(deflateSetDictionary(&zstream, (const Bytef*)dict.begin(), uInt(dict.size()))){
				case Z_OK: return ze_ok;
				default: return ze_internal_error;
			}
		}
		zip_result do_deflate(io::reader& rd, io::writer& wr, dict_type dict)
		{
			long_size_t in_bytes  = 0;
			long_size_t out_bytes = 0;

			aio::buffer<byte> bin, bout;
			bin.resize(BufferSize);
			bout.resize(BufferSize);

			for(;;){
				//flush output buffer
				if (zstream.avail_out != bout.size())	//avaliable output
				{
					if (zstream.next_out != 0){
						auto rest = block_write(wr, aio::make_range(bout.begin(), (byte*)zstream.next_out));
						auto output = aio::make_range<io::writer::iterator>(bout.begin(), rest.begin());
						out_bytes += output.size();

						if (!rest.empty())
							return zip_result{ze_stream_error, in_bytes, out_bytes };
					}

					zstream.next_out = (Bytef*)bout.begin();
					zstream.avail_out = uInt(bout.size());
				}

				if (zstream.avail_in == 0 && rd.readable())
				{
                    auto rest = rd.read(to_range(bin));
					auto readin = make_range(bin.begin(), rest.begin());
					if (readin.empty()) //read nothing, continue
						continue;

					zstream.next_in = (Bytef*)readin.begin();
					zstream.avail_in = uInt(readin.size());
					in_bytes += readin.size();
				}

				if (zstream.avail_in != 0)
				{
					switch (deflate(&zstream, Z_NO_FLUSH)){
						case Z_OK: 
						case Z_BUF_ERROR: break;
						default:
										  return zip_result{ze_internal_error, in_bytes, out_bytes};
					}
				}
				else
				{
					int ret = deflate(&zstream, Z_FINISH);

					if (ret == Z_STREAM_END)
                    {
                        auto rest = block_write(wr, aio::make_range(bout.begin(), (byte*)zstream.next_out));
						auto output = make_range<io::writer::iterator>(bout.begin(), rest.begin());
						out_bytes += output.size();
						return zip_result{rest.empty() ? ze_ok : ze_stream_error, in_bytes, out_bytes};                    
                    }
					else if (ret == Z_OK || ret == Z_BUF_ERROR)
                        continue;
                    else
						return zip_result{ze_internal_error, in_bytes, out_bytes};

				}
			}
        }
		zip_result do_deflate(io::read_map& rd, io::write_map& wr, dict_type dict)
		{
			typedef ext_heap::handle handle;

			long_size_t in_pos = 0;
			long_size_t out_pos = 0;
			long_size_t rest_in_size = rd.size();
			long_size_t rest_out_size = wr.size();
			iauto<io::read_view> rview;
			iauto<io::write_view> wview;
			range<const byte*> bin;
			range<byte*> bout;

			for(;;){

				if (rest_in_size > 0 && zstream.avail_in == 0){
					rview = rd.view_rd(handle(in_pos, in_pos + 
								std::min(ViewSize, (rest_in_size == 0 ? ViewSize : rest_in_size))));
					bin = rview.get<io::read_view>().address();
					zstream.avail_in = uInt(bin.size());
					zstream.next_in = (Bytef*)bin.begin();
					in_pos += bin.size();
					rest_in_size -= bin.size();
				}

				if (zstream.avail_out == 0){
					out_pos += bout.size();
					wview = wr.view_wr(handle(out_pos, out_pos + 
								std::min(ViewSize, (rest_out_size == 0 ? ViewSize : rest_out_size))));
					bout = wview.get<io::write_view>().address();
					zstream.avail_out = uInt(bout.size());
					zstream.next_out = (Bytef*)bout.begin();
				}

				if (zstream.avail_in != 0){
					auto res = deflate(&zstream, Z_NO_FLUSH);

					if(res == Z_OK || res == Z_BUF_ERROR) 
						continue;
					return zip_result{ze_internal_error, in_pos, out_pos + (bout.size() - zstream.avail_out)};
				}

				if(rest_in_size == 0)//finish
				{
					int ret = deflate(&zstream, Z_FINISH);

					if (ret == Z_STREAM_END)
						return zip_result{ze_ok , in_pos, out_pos + (bout.size() - zstream.avail_out)}; 
					else if (ret == Z_OK || ret == Z_BUF_ERROR)
                        continue;
                    else
						return zip_result{ze_internal_error, in_pos, out_pos + (bout.size() - zstream.avail_out)};
				}
			}
        }
        ~zip_defalter()
        {
            deflateEnd(&zstream);
        }
    };
	}
	zip_result deflate(io::reader& src, io::writer& dest, int level/* = zl_default */, 
			dict_type dict/* = dict_type()*/,heap* h /* = 0 */, int strategy_ /* = 0 */)
	{
		zip_defalter deflater(h);

		auto err = deflater.init(level, strategy_, dict);
		if (err != ze_ok)
			return zip_result{err, 0, 0};

		return deflater.do_deflate(src, dest, dict);
	}
	zip_result deflate(io::read_map& src, io::write_map& dest, int level /* = zl_default */, 
			dict_type dict /* = dict_type()*/,  heap* h /* = 0 */, int strategy_ /*= 0*/)
	{
		zip_defalter deflater(h);

		auto err = deflater.init(level, strategy_, dict);
		if (err != ze_ok)
			return zip_result{err, 0, 0};

		return deflater.do_deflate(src, dest, dict);
	}
}}
