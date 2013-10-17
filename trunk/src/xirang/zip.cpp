#include <xirang/zip.h>
#include <xirang/io/memory.h>
#include <xirang/io/exchs11n.h>
#include <xirang/deflate.h>

namespace xirang{ namespace zip{
	namespace {
		const uint16_t K_fix_part_of_local_header = 30;
		const uint32_t K_local_header_signature = 0x04034b50;
	}
	class file_reader_imp
	{
	public:
		file_reader_imp(const file_header& fh, file_type type_)
			: offset(), file(fh), header_size(), type(type_)
		{}
		long_offset_t offset;
		const file_header& file;
		uint32_t header_size;
		file_type	type;
		io::mem_archive buf;
	};
	/// file reader
	file_reader::file_reader() 
		: m_imp()
	{}
	file_reader::~file_reader() {}
	file_reader::file_reader(file_reader&& rhs)
		: m_imp(std::move(rhs.m_imp)){}
	file_reader& file_reader::operator=(file_reader rhs){
		swap(rhs);
		return *this;
	}
	void file_reader::swap(file_reader& rhs){
		m_imp.swap(rhs.m_imp);
	}
	file_reader::file_reader(const file_header& fh, file_type type)
		: m_imp(new file_reader_imp(fh, type))
	{
		AIO_PRE_CONDITION(type == ft_auto || type == ft_raw || type == ft_inflate);
		AIO_PRE_CONDITION(fh.package);
	}

	bool file_reader::valid() const{ return m_imp; }
	file_reader::operator bool() const{ return valid(); }
	file_type file_reader::type() const{
		AIO_PRE_CONDITION(valid());
		return m_imp->type;
	}

	range<byte*> file_reader::read(const range<byte*>& buf){
		AIO_PRE_CONDITION(valid());
		long_offset_t end = offset() + buf.size();
		if (end >= long_offset_t(size())) end = long_offset_t(size());
		ext_heap::handle hview(m_imp->offset, end);
		auto view = view_rd(hview);
		auto src = view.get<io::read_view>().address();
		m_imp->offset = end;
		return range<byte*>(std::copy(src.begin(), src.end(), buf.begin()), buf.end());
	}
	bool file_reader::readable() const{
		AIO_PRE_CONDITION(valid());
		return offset() < size();
	}
	long_size_t file_reader::offset() const{
		AIO_PRE_CONDITION(valid());
		return m_imp->offset;
	}
	long_size_t file_reader::size() const{
		AIO_PRE_CONDITION(valid());
		return  (m_imp->type == ft_raw)
			? m_imp->file.compressed_size
			: m_imp->file.uncompressed_size;
	}
	long_size_t file_reader::seek(long_size_t off){
		AIO_PRE_CONDITION(valid());
		m_imp->offset = std::min(off, size());
		return long_size_t(m_imp->offset);
	}
	iauto<io::read_view> file_reader::view_rd(ext_heap::handle h){
		AIO_PRE_CONDITION(valid());
		if (m_imp->header_size == 0){
			auto header_view = m_imp->file.package->view_rd(ext_heap::handle(m_imp->file.relative_offset, m_imp->file.relative_offset + K_fix_part_of_local_header));
			io::buffer_in buf(header_view.get<io::read_view>().address());
			auto loader = io::exchange::as_source(buf);
			uint32_t sig = 0;
			uint16_t fname_size(0), extra_size(0);

			loader & sig 
				//version needed to extract       2 bytes
				//general purpose bit flag        2 bytes
				//compression method              2 bytes
				//last mod file time              2 bytes
				//last mod file date              2 bytes
				//crc-32                          4 bytes
				//compressed size                 4 bytes
				//uncompressed size               4 bytes
				& io::skip_n<2 * 5 + 4 * 3>()
				//file name length                2 bytes
				& fname_size
				//extra field length              2 bytes
				& extra_size;

			if (sig != K_local_header_signature)
				AIO_THROW(data_corrupted_exception)("bad zip local header signature");

			m_imp->header_size = K_fix_part_of_local_header + fname_size + extra_size;
			if (m_imp->header_size > 0xffff)
				AIO_THROW(data_corrupted_exception)("bad zip local header size");
		}
		// ft_raw
		AIO_PRE_CONDITION(h.size() <= m_imp->file.compressed_size);

		auto data_offset = m_imp->file.relative_offset + m_imp->header_size;
		ext_heap::handle realh(h.begin() + data_offset, h.end() + data_offset);
		return m_imp->file.package->view_rd(realh);

		//TODO:
		//ft_inflate:
	}

#if 0
	/// zip reader
	reader::reader(io::read_map& ar);
	reader::reader();
	reader::~reader();
	reader::reader(reader&& rhs);
	reader& reader::operator=(reader&& rhs);
	void reader::swap(reader& rhs);

	bool reader::valid() const;
	reader::operator bool() const;

	bool reader::exists(const file_path& name) const;
	const file_header& reader::find(const file_path& name) const;

	range<reader::iterator> reader::items() const;
	range<reader::iterator> reader::items(const file_path& dir) const;

	/// zip reader_writer
	reader_writer::reader_writer();
	reader_writer::~reader_writer();
	reader_writer::reader_writer(iref<io::read_map, io::write_map, io::ioctrl> ar);
	reader_writer::reader_writer(reader_writer&& rhs);
	reader_writer& reader_writer::operator=(reader_writer&& rhs);
	void reader_writer::swap(reader_writer& rhs);

	bool reader_writer::valid() const;
	reader_writer::operator bool() const;

	bool reader_writer::exists(const file_path& name) const;
	const file_header& reader_writer::find(const file_path& name) const;

	file_reader reader_writer::open(const file_path& name, file_type type = ft_auto) const;
	range<reader_writer::iterator> reader_writer::items() const;
	range<reader_writer::iterator> reader_writer::items(const file_path& dir) const;

	bool reader_writer::append(io::reader& ar, const file_path& name);
	bool reader_writer::append(io::reader& ar, const file_header& h, file_type type);
#endif
}}

