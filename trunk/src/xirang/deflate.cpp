#include <xirang/deflate.h>
#include <zlib.h>
#include <xirang/io/memory.h>
#include <xirang/io/adaptor.h>

namespace xirang{ namespace zip{
	namespace{
		const size_t K_BufferSize = 64 * 1024;
		const long_size_t K_UncompressedViewSize = 1024 * 1024;
		const long_size_t K_CompressedViewSize = 64 * 1024;
	}

	uint32_t crc32_init(){
		return ::crc32(0L, Z_NULL, 0);
	}
	uint32_t crc32(io::reader& src, uint32_t crc /* = crc32_init()*/){
		buffer<byte> bin;
		bin.resize(K_BufferSize);
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
			auto rview = src.view_rd(ext_heap::handle(in_pos, in_pos + std::min(K_UncompressedViewSize, rest_in_size)));
			auto rng = rview.get<io::read_view>().address();
			crc = crc32(rng, crc);
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


	}

	class inflate_reader_imp{
		public:
			inflate_reader_imp(io::reader& src_, long_size_t uncompressed_size_, dict_type dict_, heap* h)
				: stream_src(&src_), dict(dict_)
				, uncompressed_size(uncompressed_size_)
			{
				init_(dict_, h);
			}
			inflate_reader_imp(io::read_map& src_, long_size_t uncompressed_size_, dict_type dict_, heap* h)
				: map_src(&src_), dict(dict_)
				, uncompressed_size(uncompressed_size_)
			{
				init_(dict_, h);
			}
			void init_(dict_type dict_, heap* h){
				if (h){
					zstream.zalloc = zip_malloc;
					zstream.zfree = zip_free;
					zstream.opaque = h;
				}
				if (inflateInit2(&zstream, -MAX_WBITS) != Z_OK)
					AIO_THROW(inflate_exception)("inflateInit2 failed");
			}
			~inflate_reader_imp(){
				inflateEnd(&zstream);
			}

			range<byte*> read(const range<byte*>& buf)
			{
				zstream.avail_out = uInt(buf.size());
				zstream.next_out = (Bytef*)buf.begin();

				for(;zstream.avail_out != 0;){
					if (zstream.avail_in == 0){
						new_input_();
					}

					auto res = ::inflate(&zstream, Z_NO_FLUSH);
					switch(res)
					{
						case Z_NEED_DICT:
							{
								if (dict.empty()) 
									AIO_THROW(inflate_exception)("Need dictionary");
								auto dic_err = inflateSetDictionary(&zstream, (Bytef*)dict.begin(), dict.size());
								if (dic_err != Z_OK) 
									AIO_THROW(inflate_exception)("Bad dictionary");
							}
						case Z_OK:
							break;
						case Z_DATA_ERROR:
							AIO_THROW(inflate_exception)("Z_DATA_ERROR");
						case Z_MEM_ERROR:
							AIO_THROW(inflate_exception)("Z_MEM_ERROR");
						case Z_STREAM_END:
							uncompressed_size = zstream.total_out;
							return range<byte*>((buf.size() - zstream.avail_out) + buf.begin(), buf.end());
						default:
							AIO_THROW(inflate_exception)("zlib internal error");
					}
				}
				return range<byte*>(buf.end() - zstream.avail_out, buf.end());
			}
			void new_input_(){
				if (map_src){
					long_size_t rest_in_size = map_src->size() - zstream.total_in;
					if (rest_in_size > 0){
						typedef ext_heap::handle handle;
						handle hin(zstream.total_in,  zstream.total_in + std::min(K_UncompressedViewSize, rest_in_size));
						rview = map_src->view_rd(hin);
						auto bin = rview.get<io::read_view>().address();
						zstream.avail_in = uInt(bin.size());
						zstream.next_in = (Bytef*)bin.begin();
					}
				}
				else if(stream_src->readable()){
					buf.resize(K_UncompressedViewSize);
					auto res = stream_src->read(to_range(buf));
					buf.resize(buf.size() - res.size());
					zstream.avail_in = uInt(buf.size());
					zstream.next_in = (Bytef*)buf.begin();
				}
			}

			io::read_map* map_src = 0;
			io::reader* stream_src = 0;
			buffer<byte> buf;
			dict_type dict;
			z_stream zstream = z_stream();
			long_size_t uncompressed_size = long_size_t(-1);
			iauto<io::read_view> rview;

	};

	inflate_reader::inflate_reader(){}
	inflate_reader::~inflate_reader(){}

	inflate_reader::inflate_reader(inflate_reader&& rhs)
		: m_imp(std::move(rhs.m_imp))
	{}
	inflate_reader& inflate_reader::operator=(inflate_reader rhs){
		swap(rhs);
		return *this;
	}
	void inflate_reader::swap(inflate_reader& rhs){
		m_imp.swap(rhs.m_imp);
	}

	inflate_reader::inflate_reader(io::reader& src, long_size_t uncompressed_size,dict_type dict /*= dict_type() */,heap* h /* = 0 */)
		: m_imp(new inflate_reader_imp(src, uncompressed_size, dict, h))
	{
	}
	inflate_reader::inflate_reader(io::read_map& src, long_size_t uncompressed_size,dict_type dict /*= dict_type() */,heap* h /* = 0 */)
		: m_imp(new inflate_reader_imp(src, uncompressed_size, dict, h))
	{
	}
	bool inflate_reader::valid() const{
		return m_imp;
	}
	inflate_reader::operator bool() const{
		return valid();
	}
	range<byte*> inflate_reader::read(const range<byte*>& buf){
		AIO_PRE_CONDITION(valid());
		return m_imp->read(buf);
	}
	bool inflate_reader::readable() const{
		AIO_PRE_CONDITION(valid());
		return offset() < m_imp->uncompressed_size;
	}
	long_size_t inflate_reader::offset() const{
		AIO_PRE_CONDITION(valid());
		return m_imp->zstream.total_out;
	}
	long_size_t inflate_reader::size() const{
		AIO_PRE_CONDITION(valid());
		return m_imp->uncompressed_size;
	}

	long_size_t inflate_reader::seek(long_size_t off){
		AIO_PRE_CONDITION(valid());
		if (off <= offset())
			return offset();

		io::mem_archive mar;
		io::null dest;
		io::copy_data<io::reader, io::writer>(*this, dest, off - offset());
		return offset();
	}
	long_size_t inflate_reader::compressed_size() const{
		AIO_PRE_CONDITION(valid() && !readable());
		return m_imp->zstream.total_in;
	}


	class deflate_writer_imp
	{
		public:
			deflate_writer_imp(io::writer& dest_, int level_, dict_type dict_, heap* h, int strategy_)
				: stream_dest(&dest_)
			{
				init_(level_, dict_, h, strategy_);
			}
			deflate_writer_imp(io::write_map& dest_, int level_, dict_type dict_, heap* h, int strategy_)
				: map_dest(&dest_)
			{
				init_(level_, dict_, h, strategy_);
			}
			void init_(int level_, dict_type dict_, heap* h, int strategy_){
				if (h){
					zstream.zalloc = zip_malloc;
					zstream.zfree = zip_free;
					zstream.opaque = h;
				}
				if(deflateInit2(&zstream, level_, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, strategy_) != Z_OK)
					AIO_THROW(deflate_exception)("deflateInit2 failed");

				if (!dict_.empty() && (deflateSetDictionary(&zstream, (const Bytef*)dict_.begin(), uInt(dict_.size())) != Z_OK))
					AIO_THROW(deflate_exception)("deflateSetDictionary failed");
			}

			range<const byte*> write(const range<const byte*>& buf){
				AIO_PRE_CONDITION(!finished);
				AIO_PRE_CONDITION(zstream.avail_in == 0);
				zstream.next_in = (Bytef*)buf.begin();
				zstream.avail_in = uInt(buf.size());

				for(;zstream.avail_in != 0;){
					if (zstream.avail_out ==0 )
						new_buffer_();

					switch (deflate(&zstream, Z_NO_FLUSH)){
						case Z_OK: 
						case Z_BUF_ERROR: 
							break;
						default:
							AIO_THROW(deflate_exception)("deflate with Z_NO_FLUSH failed");
					}
				}
				return range<const byte*>(buf.end(), buf.end());
			}
			void finish(){
				AIO_PRE_CONDITION(!finished);
				for (;;){
					if (zstream.avail_out ==0 )
						new_buffer_();

					int ret = deflate(&zstream, Z_FINISH);

					if (ret == Z_STREAM_END)
					{
						if (stream_dest){
							stream_dest->write(make_range(buf.begin(), reinterpret_cast<byte*>(zstream.next_out)));
							buffer<byte>().swap(buf);
						}
						break;
					}
					else if (ret == Z_OK || ret == Z_BUF_ERROR)
						continue;
					else
						AIO_THROW(deflate_exception)("deflate with Z_FINISH failed");
				}
				wview.reset();
				finished = true;
			}
			void new_buffer_(){
				if (map_dest){
					wview = map_dest->view_wr(ext_heap::handle(zstream.total_out, zstream.total_out + K_CompressedViewSize));
					if (wview.get<io::write_view>().address().size() != K_CompressedViewSize)
						AIO_THROW(deflate_exception)("Failed to request write view");

					auto addr  = wview.get<io::write_view>().address();
					zstream.next_out = (Bytef*)addr.begin();
					zstream.avail_out = uInt(addr.size());
				}
				else{
					stream_dest->write(to_range(buf));
					buf.resize(K_CompressedViewSize);
					zstream.next_out = (Bytef*)buf.begin();
					zstream.avail_out = uInt(buf.size());
				}
			}
			void sync(){
				if (map_dest)	
					map_dest->sync();
				else{
					stream_dest->sync();
				}
			}

			~deflate_writer_imp(){
				deflateEnd(&zstream);
			};

			io::write_map* map_dest = 0;
			io::writer* stream_dest = 0;
			buffer<byte> buf;
			z_stream zstream = z_stream();
			iauto<io::write_view> wview;
			bool finished = false;
	};
	deflate_writer::deflate_writer(){}
	deflate_writer::~deflate_writer(){
		if (!finished())
			finish();
	}
	deflate_writer::deflate_writer(deflate_writer&& rhs) : m_imp(std::move(rhs.m_imp)){}
	deflate_writer& deflate_writer::operator=(deflate_writer rhs){
		swap(rhs);
		return *this;
	}
	void deflate_writer::swap(deflate_writer& rhs){
		m_imp.swap(rhs.m_imp);
	}

	deflate_writer::deflate_writer(io::writer& dest,
			int level/* = zl_default */, dict_type dict /* = dict_type() */,heap* h /* = 0 */, int strategy_ /* = zs_default */)
		: m_imp(new deflate_writer_imp(dest, level, dict, h, strategy_))
	{}
	deflate_writer::deflate_writer(io::write_map& dest,
			int level/* = zl_default */, dict_type dict /* = dict_type() */,heap* h /* = 0 */, int strategy_ /* = zs_default */)
		: m_imp(new deflate_writer_imp(dest, level, dict, h, strategy_))
	{}
	bool deflate_writer::valid() const{
		return m_imp;
	}
	deflate_writer::operator bool() const{
		return valid();
	}

	range<const byte*> deflate_writer::write(const range<const byte*>& buf){
		AIO_PRE_CONDITION(valid() && !finished());
		return m_imp->write(buf);
	}
	bool deflate_writer::writable() const{
		AIO_PRE_CONDITION(valid() && !finished());
		return true;
	}
	long_size_t deflate_writer::offset() const{
		return size();
	}
	long_size_t deflate_writer::size() const{
		AIO_PRE_CONDITION(valid());
		return m_imp->zstream.total_out;
	}
	void deflate_writer::sync(){
		AIO_PRE_CONDITION(valid());
		m_imp->sync();
	}
	long_size_t deflate_writer::uncompressed_size() const{
		AIO_PRE_CONDITION(valid());
		return m_imp->zstream.total_in;
	}

	bool deflate_writer::finished() const{
		AIO_PRE_CONDITION(valid());
		return m_imp->finished;
	}
	void deflate_writer::finish() {
		AIO_PRE_CONDITION(valid() && !finished());
		m_imp->finish();
	}

	zip_result inflate(io::reader& src, io::writer& dest, dict_type dict,heap* h ){
		inflate_reader reader(src, long_size_t(-1), dict, h);
		iref<io::reader> from(reader);
		auto size = io::copy_data(from.get<io::reader>(), dest);
		return zip_result{ze_ok, reader.compressed_size(), size};
	}
	zip_result inflate(io::read_map& src, io::write_map& dest, dict_type dict,  heap* h){
		inflate_reader reader(src, long_size_t(-1), dict, h);
		iref<io::reader> from(reader);
		auto size = io::copy_data(from.get<io::reader>(), dest);
		return zip_result{ze_ok, src.size(), size};
	}

	zip_result deflate(io::reader& src, io::writer& dest, int level, dict_type dict, heap* h, int strategy_){
		deflate_writer writer(dest, level, dict, h, strategy_);
		iref<io::writer> to(writer);

		io::copy_data(src, to.get<io::writer>());
		writer.finish();
		return zip_result{ze_ok, writer.uncompressed_size(), writer.size()};
	}
	zip_result deflate(io::read_map& src, io::write_map& dest, int level, dict_type dict,  heap* h, int strategy_){
		deflate_writer writer(dest, level, dict, h, strategy_);
		iref<io::writer> to(writer);

		io::copy_data(src, to.get<io::writer>());
		writer.finish();
		return zip_result{ze_ok, src.size(), writer.size()};
	}

}}

