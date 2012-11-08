#include <aio/common/archive/mem_archive.h>
namespace aio{ namespace archive {

	bool in_size_t_range(long_size_t n){
		return (n & ~long_size_t(size_t(-1))) == 0;
	}

	/// buffer_in
	buffer_in::buffer_in(const buffer<byte>& buf)
		: m_pos(0), m_data(buf)
	{ }

	buffer_in::iterator buffer_in::read(const range<buffer_in::iterator>& buf)
	{
		buffer<byte>::const_iterator sitr = m_data.begin() + m_pos;
		buffer<byte>::iterator ditr = buf.begin();
		for (; sitr != m_data.end() && ditr != buf.end();++sitr, ++ditr)
		{
			*ditr = *sitr;
		}
		m_pos = sitr - m_data.begin();
		return ditr;
	}

	bool buffer_in::readable() const
	{
		return m_pos < m_data.size();
	}

	bool buffer_in::viewable() const{
		return false;
	}

	const_view buffer_in::view_rd(ext_heap::handle) const {
		return const_view();
	}

	long_size_t buffer_in::offset() const { return m_pos;}
	long_size_t buffer_in::size() const { return m_data.size();}
	long_size_t buffer_in::seek(long_size_t offset)
	{
		AIO_PRE_CONDITION(in_size_t_range(offset));
		m_pos = (size_t)std::min(size(), offset);
		return m_pos;
	}

	const buffer<byte> & buffer_in::data() { return m_data;}

	/// buffer_out
	buffer_out::buffer_out(buffer<byte>& buf)
		: m_pos(0), m_data(buf)
	{ }

	buffer_out::const_iterator buffer_out::write(const range<buffer_out::const_iterator>& r)
	{
		if (!r.empty())
		{
			if (m_pos > m_data.size())
				m_data.resize(m_pos);

			buffer<byte>::iterator ditr = m_data.begin() + m_pos;
			buffer<byte>::const_iterator sitr = r.begin();
			for (; sitr != r.end() && ditr != m_data.end(); ++sitr, ++ditr)
			{
				*ditr = *sitr;
			}
			if (sitr != r.end())
			{
				m_data.append(make_range(sitr, r.end()));
				m_pos = m_data.size();
			}
			else
				m_pos = ditr - m_data.begin();
		}
		return r.end();

	}

	long_size_t buffer_out::truncate(long_size_t size)
	{
		AIO_PRE_CONDITION(in_size_t_range(size));

		m_pos = std::min(m_pos, (size_t)size);
		m_data.resize((size_t)size);
		return size;
	}

	bool buffer_out::writable() const { return true;}

	bool buffer_out::viewable() const{
		return false;
	}

	view buffer_out::view_wr(ext_heap::handle) {
		return view();
	}


	void buffer_out::sync() {}

	long_size_t buffer_out::offset() const { return m_pos;}
	long_size_t buffer_out::size() const { return m_data.size();}
	long_size_t buffer_out::seek(long_size_t offset)
	{
		AIO_PRE_CONDITION(in_size_t_range(offset));
		m_pos = (size_t)offset;
		return m_pos;
	}

	buffer<byte> & buffer_out::data() { return m_data;}

	/// buffer_io

	buffer_io::buffer_io(buffer<byte>& buf)
		: m_pos(0), m_data(buf)
	{ }

	buffer_io::iterator buffer_io::read(const range<buffer_io::iterator>& buf)
	{
		if (m_pos > m_data.size())
			return buf.begin();

		buffer<byte>::iterator sitr = m_data.begin() + m_pos;
		buffer<byte>::iterator ditr = buf.begin();
		for (; sitr != m_data.end() && ditr != buf.end();++sitr, ++ditr)
		{
			*ditr = *sitr;
		}
		m_pos = sitr - m_data.begin();
		return ditr;
	}

	bool buffer_io::readable() const
	{
		return m_pos < m_data.size();
	}

	bool buffer_io::viewable() const{
		return false;
	}

	const_view buffer_io::view_rd(ext_heap::handle) const {
		return const_view();
	}

	view buffer_io::view_wr(ext_heap::handle) {
		return view();
	}

	buffer_io::const_iterator buffer_io::write(const range<buffer_io::const_iterator>& r)
	{
		if (!r.empty())
		{
			if (m_pos > m_data.size())
				m_data.resize(m_pos);

			buffer<byte>::iterator ditr = m_data.begin() + m_pos;
			buffer<byte>::const_iterator sitr = r.begin();
			for (; sitr != r.end() && ditr != m_data.end(); ++sitr, ++ditr)
			{
				*ditr = *sitr;
			}
			if (sitr != r.end())
			{
				m_data.append(make_range(sitr, r.end()));
				m_pos = m_data.size();
			}
			else
				m_pos = ditr - m_data.begin();
		}
		return r.end();

	}

	long_size_t buffer_io::truncate(long_size_t size)
	{
		AIO_PRE_CONDITION(in_size_t_range(size));
		m_pos = std::min(m_pos, (size_t)size);
		m_data.resize((size_t)size);
		return size;
	}

	bool buffer_io::writable() const { return true;}
	void buffer_io::sync() {}

	long_size_t buffer_io::offset() const { return m_pos;}
	long_size_t buffer_io::size() const { return m_data.size(); }
	long_size_t buffer_io::seek(long_size_t offset) { 
		AIO_PRE_CONDITION(in_size_t_range(offset));
		return m_pos = (size_t)offset;
	}

	buffer<byte> & buffer_io::data() { return m_data;}

	/// mem_read_archive
	mem_write_archive::mem_write_archive() : buffer_out(m_data){}
	mem_read_write_archive::mem_read_write_archive() : buffer_io(m_data){}
	mem_read_write_archive::mem_read_write_archive(const buffer<byte>& buf) : buffer_io(m_data) { m_data = buf; }

}
namespace archive_new{
	bool in_size_t_range(long_size_t n){
		return (n & ~long_size_t(size_t(-1))) == 0;
	}

	/// buffer_in
	buffer_in::buffer_in(const buffer<byte>& buf)
		: m_pos(0), m_data(buf)
	{ }

	range<buffer_in::iterator> buffer_in::read(const range<buffer_in::iterator>& buf)
	{
		buffer<byte>::const_iterator sitr = m_data.begin() + m_pos;
		buffer<byte>::iterator ditr = buf.begin();
		for (; sitr != m_data.end() && ditr != buf.end();++sitr, ++ditr)
		{
			*ditr = *sitr;
		}
		m_pos = sitr - m_data.begin();
		return range<buffer_in::iterator>(ditr, buf.end());
	}

	bool buffer_in::readable() const
	{
		return m_pos < m_data.size();
	}

	unique_ptr<read_view> buffer_in::view_rd(ext_heap::handle h) const {
		AIO_PRE_CONDITION((size_t)h.begin <= (size_t)h.end);
		AIO_PRE_CONDITION((size_t)h.end <= size());
		return unique_ptr<buffer_rd_view>(new buffer_rd_view(range<const byte*>(m_data.begin() + h.begin, m_data.begin() + h.end)));
	}

	long_size_t buffer_in::offset() const { return m_pos;}
	long_size_t buffer_in::size() const { return m_data.size();}
	long_size_t buffer_in::seek(long_size_t offset)
	{
		AIO_PRE_CONDITION(in_size_t_range(offset));
		m_pos = (size_t)std::min(size(), offset);
		return m_pos;
	}

	const buffer<byte> & buffer_in::data() { return m_data;}

	/// buffer_out
	buffer_out::buffer_out(buffer<byte>& buf)
		: m_pos(0), m_data(buf)
	{ }

	range<const byte*> buffer_out::write(const range<const byte*>& r)
	{
		if (!r.empty())
		{
			if (m_pos > m_data.size())
				m_data.resize(m_pos);

			buffer<byte>::iterator ditr = m_data.begin() + m_pos;
			buffer<byte>::const_iterator sitr = r.begin();
			for (; sitr != r.end() && ditr != m_data.end(); ++sitr, ++ditr)
			{
				*ditr = *sitr;
			}
			if (sitr != r.end())
			{
				m_data.append(make_range(sitr, r.end()));
				m_pos = m_data.size();
			}
			else
				m_pos = ditr - m_data.begin();
		}
		return range<const byte*>(r.end(), r.end());

	}

	long_size_t buffer_out::truncate(long_size_t size)
	{
		AIO_PRE_CONDITION(in_size_t_range(size));

		m_pos = std::min(m_pos, (size_t)size);
		m_data.resize((size_t)size);
		return size;
	}

	bool buffer_out::writable() const { return true;}

	unique_ptr<write_view> buffer_out::view_wr(ext_heap::handle h) {
		AIO_PRE_CONDITION((size_t)h.begin <= (size_t)h.end);
		AIO_PRE_CONDITION((size_t)h.end <= size());
		return unique_ptr<buffer_wr_view>(new buffer_wr_view(range<byte*>(m_data.begin() + h.begin, m_data.begin() + h.end)));
	}


	void buffer_out::sync() {}

	long_size_t buffer_out::offset() const { return m_pos;}
	long_size_t buffer_out::size() const { return m_data.size();}
	long_size_t buffer_out::seek(long_size_t offset)
	{
		AIO_PRE_CONDITION(in_size_t_range(offset));
		m_pos = (size_t)offset;
		return m_pos;
	}

	buffer<byte> & buffer_out::data() { return m_data;}

	/// buffer_io

	buffer_io::buffer_io(buffer<byte>& buf)
		: m_pos(0), m_data(buf)
	{ }

	range<byte*> buffer_io::read(const range<byte*>& buf)
	{
		if (m_pos > m_data.size())
			return buf;

		buffer<byte>::iterator sitr = m_data.begin() + m_pos;
		buffer<byte>::iterator ditr = buf.begin();
		for (; sitr != m_data.end() && ditr != buf.end();++sitr, ++ditr)
		{
			*ditr = *sitr;
		}
		m_pos = sitr - m_data.begin();
		return range<byte*>(ditr, buf.end());
	}

	bool buffer_io::readable() const
	{
		return m_pos < m_data.size();
	}

	unique_ptr<read_view> buffer_io::view_rd(ext_heap::handle h) const{
		AIO_PRE_CONDITION((size_t)h.begin <= (size_t)h.end);
		AIO_PRE_CONDITION((size_t)h.end <= size());
		return unique_ptr<read_view>(new buffer_rd_view(range<const byte*>(m_data.begin() + h.begin, m_data.begin() + h.end)));
	}

	unique_ptr<write_view> buffer_io::view_wr(ext_heap::handle h) {
		AIO_PRE_CONDITION((size_t)h.begin <= (size_t)h.end);
		AIO_PRE_CONDITION((size_t)h.end <= size());
		return unique_ptr<write_view>(new buffer_wr_view(range<byte*>(m_data.begin() + h.begin, m_data.begin() + h.end)));
	}

	range<const byte*> buffer_io::write(const range<const byte*>& r)
	{
		if (!r.empty())
		{
			if (m_pos > m_data.size())
				m_data.resize(m_pos);

			buffer<byte>::iterator ditr = m_data.begin() + m_pos;
			buffer<byte>::const_iterator sitr = r.begin();
			for (; sitr != r.end() && ditr != m_data.end(); ++sitr, ++ditr)
			{
				*ditr = *sitr;
			}
			if (sitr != r.end())
			{
				m_data.append(make_range(sitr, r.end()));
				m_pos = m_data.size();
			}
			else
				m_pos = ditr - m_data.begin();
		}
		return range<const byte*>(r.end(), r.end());

	}

	long_size_t buffer_io::truncate(long_size_t size)
	{
		AIO_PRE_CONDITION(in_size_t_range(size));
		m_pos = std::min(m_pos, (size_t)size);
		m_data.resize((size_t)size);
		return size;
	}

	bool buffer_io::writable() const { return true;}
	void buffer_io::sync() {}

	long_size_t buffer_io::offset() const { return m_pos;}
	long_size_t buffer_io::size() const { return m_data.size(); }
	long_size_t buffer_io::seek(long_size_t offset) { 
		AIO_PRE_CONDITION(in_size_t_range(offset));
		return m_pos = (size_t)offset;
	}

	buffer<byte> & buffer_io::data() { return m_data;}

	/// mem_read_archive
	mem_write_archive::mem_write_archive() : buffer_out(m_data){}
	mem_read_write_archive::mem_read_write_archive() : buffer_io(m_data){}
	mem_read_write_archive::mem_read_write_archive(const buffer<byte>& buf) : buffer_io(m_data) { m_data = buf; }
}}

