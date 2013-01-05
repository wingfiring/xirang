#include <aio/common/zip.h>
#include <zlib.h>

namespace aio{ namespace zip{
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

		const size_t BufferSize = 64 * 1024;
		const long_size_t ViewSize = 1024 * 1024;

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
									auto dic_err = inflateSetDictionary(&zstream, (Bytef*)dict.begin(), dict.size());
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
				long_size_t in_bytes  = 0;
				long_size_t out_bytes = 0;
				typedef ext_heap::handle handle;

				long_size_t in_pos = 0;
				long_size_t out_pos = 0;
				long_size_t rest_in_size = rd.size();
				long_size_t rest_out_size = wr.size();

				while (rest_in_size > 0) { 
					auto rview = rd.view_rd(handle(in_pos, in_pos + std::min(ViewSize, rest_in_size)));
					auto bin = rview->address();

					in_pos += bin.size();
					rest_in_size -= bin.size();

					zstream.avail_in = bin.size();
					zstream.next_in = (Bytef*)bin.begin();

					do {
						auto wview = wr.view_wr(handle(out_pos, out_pos + std::min(ViewSize, (rest_out_size == 0 ? ViewSize : rest_out_size))));
						auto bout = wview->address();

						zstream.avail_out = uInt(bout.size());
						zstream.next_out = (Bytef*)bout.begin();

						auto res = ::inflate(&zstream, Z_NO_FLUSH);
						in_bytes += bin.size() - zstream.avail_in;
						out_bytes += bout.size()- zstream.avail_out;

						out_pos += bout.size();
						if (rest_out_size > 0) {
							AIO_PRE_CONDITION(rest_out_size >= bout.size());
							rest_out_size -= bout.size();
						}
						switch(res)
						{
							case Z_NEED_DICT:
								{
									if (dict.empty()) return zip_result{ze_need_dict, in_bytes, out_bytes};
									auto dic_err = inflateSetDictionary(&zstream, (Bytef*)dict.begin(), dict.size());
									if (dic_err != Z_OK) return zip_result{ze_data_error, in_bytes, out_bytes};
								}
								break;
							case Z_DATA_ERROR:
								return zip_result{ze_data_error, in_bytes, out_bytes};
							case Z_MEM_ERROR:
								return zip_result{ze_mem_error, in_bytes, out_bytes};
							case Z_STREAM_END:
								return zip_result{ze_ok, in_bytes, out_bytes};
						}
					} while(zstream.avail_out == 0);
				}
				return zip_result{ze_unfinished_data, in_bytes, out_bytes};
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


	zip_result deflate(io::reader& src, io::writer& dest, int level/* = zl_default */, 
			dict_type dict/* = dict_type()*/,heap* h /* = 0 */, int strategy_ /* = 0 */)
	{
		return zip_result{ze_ok, 0, 0};
	}
	zip_result deflate(io::read_map& src, io::write_map& dest, int level /* = zl_default */, 
			dict_type dict /* = dict_type()*/,  heap* h /* = 0 */, int strategy_ /*= 0*/)
	{
		return zip_result{ze_ok, 0, 0};
	}
}}

