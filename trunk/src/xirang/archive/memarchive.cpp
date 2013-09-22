#include <xirang/io/memory.h>
namespace aio{ namespace io{

	bool in_size_t_range(long_size_t n){
		return (n & ~long_size_t(size_t(-1))) == 0;
	}

	/// buffer_in
	buffer_in::buffer_in(const buffer<byte>& buf)
		: m_pos(0), m_data(buf)
	{ }

	buffer_in::buffer_in(const range<const byte*>& buf)
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

	buffer_rd_view buffer_in::view_rd(ext_heap::handle h) const {
		AIO_PRE_CONDITION((size_t)h.begin() <= (size_t)h.end());
		AIO_PRE_CONDITION((size_t)h.end() <= size());
		return buffer_rd_view(range<const byte*>(m_data.begin() + h.begin(), m_data.begin() + h.end()));
	}

	long_size_t buffer_in::offset() const { return m_pos;}
	long_size_t buffer_in::size() const { return m_data.size();}
	long_size_t buffer_in::seek(long_size_t offset)
	{
		AIO_PRE_CONDITION(in_size_t_range(offset));
		m_pos = (size_t)std::min(size(), offset);
		return m_pos;
	}

	range<const byte*> buffer_in::data() const { return m_data;}

	/// fixed_buffer_io
	fixed_buffer_io::fixed_buffer_io(const range<byte*>& buf)
		: m_pos(0), m_data(buf)
	{ }

	range<byte*> fixed_buffer_io::read(const range<byte*>& buf){
		auto sitr = m_data.begin() + m_pos;
		auto ditr = buf.begin();
		for (; sitr != m_data.end() && ditr != buf.end();++sitr, ++ditr)
		{
			*ditr = *sitr;
		}
		m_pos = sitr - m_data.begin();
		return range<fixed_buffer_io::iterator>(ditr, buf.end());
	}
	bool fixed_buffer_io::readable() const{
		return m_pos < m_data.size();
	}
	range<const byte*> fixed_buffer_io::write(const range<const byte*>& r)
	{
		auto ditr = m_data.begin() + m_pos;
		auto sitr = r.begin();
		for (; sitr != r.end() && ditr != m_data.end(); ++sitr, ++ditr)
		{
			*ditr = *sitr;
		}
		return range<const byte*>(sitr, r.end());

	}

	bool fixed_buffer_io::writable() const { 
		return m_pos < m_data.size();
	}

	long_size_t fixed_buffer_io::truncate(long_size_t size)
	{
		AIO_PRE_CONDITION(in_size_t_range(size));
		return unknow_size;
	}

	buffer_wr_view fixed_buffer_io::view_wr(ext_heap::handle h) {
		AIO_PRE_CONDITION((size_t)h.begin() <= (size_t)h.end());
		AIO_PRE_CONDITION((size_t)h.end() <= size());
		return buffer_wr_view(range<byte*>(m_data.begin() + h.begin(), m_data.begin() + h.end()));
	}
	buffer_rd_view fixed_buffer_io::view_rd(ext_heap::handle h) const{
		AIO_PRE_CONDITION((size_t)h.begin() <= (size_t)h.end());
		AIO_PRE_CONDITION((size_t)h.end() <= size());
		return buffer_rd_view(range<const byte*>(m_data.begin() + h.begin(), m_data.begin() + h.end()));
	}


	void fixed_buffer_io::sync() {}

	long_size_t fixed_buffer_io::offset() const { return m_pos;}
	long_size_t fixed_buffer_io::size() const { return m_data.size();}
	long_size_t fixed_buffer_io::seek(long_size_t offset)
	{
		AIO_PRE_CONDITION(in_size_t_range(offset));
		if (offset >= size())
			offset = size();
		m_pos = (size_t)offset;
		return m_pos;
	}

	range<byte*> fixed_buffer_io::data() const { return m_data;}


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

	buffer_wr_view buffer_out::view_wr(ext_heap::handle h) {
		AIO_PRE_CONDITION((size_t)h.begin() <= (size_t)h.end());
		AIO_PRE_CONDITION((size_t)h.end() <= size());
		return buffer_wr_view(range<byte*>(m_data.begin() + h.begin(), m_data.begin() + h.end()));
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

	buffer_rd_view buffer_io::view_rd(ext_heap::handle h) const{
		AIO_PRE_CONDITION((size_t)h.begin() <= (size_t)h.end());
		AIO_PRE_CONDITION((size_t)h.end() <= size());
		return buffer_rd_view(range<const byte*>(m_data.begin() + h.begin(), m_data.begin() + h.end()));
	}

	buffer_wr_view buffer_io::view_wr(ext_heap::handle h) {
		AIO_PRE_CONDITION((size_t)h.begin() <= (size_t)h.end());
		if ((size_t)h.end() > size())
			m_data.resize((size_t)h.end());
		return buffer_wr_view(range<byte*>(m_data.begin() + h.begin(), m_data.begin() + h.end()));
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
	mem_writer::mem_writer() : buffer_out(m_data){}
	mem_archive::mem_archive() : buffer_io(m_data){}
	mem_archive::mem_archive(const buffer<byte>& buf) : buffer_io(m_data) { m_data = buf; }

	/// zero
	zero::zero(): m_pos(0){}
	range<byte*> zero::read(const range<byte*>& buf){
		std::fill(buf.begin(), buf.end(), byte(0));
		m_pos += buf.size();
		return range<byte*>(buf.end(), buf.end());
	}
	bool zero::readable() const { return true;}

	long_size_t zero::offset() const{ return m_pos;}
	long_size_t zero::size() const{ return unknow_size;}
	long_size_t zero::seek(long_size_t off){ return m_pos = off;}

	/// null
	null::null() : m_pos(0){}

	range<const byte*> null::write(const range<const byte*>& r){
		m_pos += r.size();
		return range<const byte*>(r.end(), r.end());
	}
	bool null::writable() const{ return true;}
	void null::sync() {}

	long_size_t null::offset() const{ return m_pos;}
	long_size_t null::size() const{ return unknow_size;}
	long_size_t null::seek(long_size_t off){
		return m_pos = off;
	} 
	long_size_t null::truncate(long_size_t ){ return unknow_size;}
}}

