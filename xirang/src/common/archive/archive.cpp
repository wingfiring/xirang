#include <aio/common/iarchive.h>
namespace aio{ namespace archive
{
	view_imp::~view_imp(){}

	const_view::operator bool() const 	{ return valid(); }

	bool const_view::valid() const		{ return m_imp != 0;}

	const void* const_view::address() const	{
		AIO_PRE_CONDITION(valid());
		return m_imp->address();
	}

	ext_heap::handle const_view::handle() const {
		AIO_PRE_CONDITION(valid());
		return m_imp->handle();
	}

	void const_view::swap(const_view& rhs) {
		std::swap(m_imp, rhs.m_imp);
	}

	const_view::const_view(view_imp* imp) : m_imp(imp) {}

	const_view::const_view(rv<const_view>& other)
		:m_imp(other.m_imp)
	{
		other.m_imp = 0;
	}
	rv<const_view>& const_view::move()
	{
		return *reinterpret_cast< rv<const_view>* >(this);  
	}

	const_view::~const_view(){
		if (valid())
			m_imp->destroy();
	}

	view::view(view_imp* imp) : const_view(imp) {}
	view::view(rv<view>& mr)
		:const_view(mr.m_imp)
	{
		mr.m_imp = 0;
	}
	rv<view>& view::move()
	{
		return *reinterpret_cast< rv<view>* >(this);  
	}

	void* view::address() const{
		AIO_PRE_CONDITION(valid());
		return m_imp->address();
	}
	void view::swap(view& rhs){
		std::swap(m_imp, rhs.m_imp);
	}

	reader::~reader() {}
	writer::~writer() {}
	sequence::~sequence() {}
	forward::~forward() {}
	random::~random() {}
	iarchive::~iarchive(){}

    any iarchive::getopt(int /*id*/, const any & /*optdata = any() */) const 
    {
        return any();
    }
    any iarchive::setopt(int /*id*/, const any & /*indata*/,  const any & /*optdata = any()*/)
    {
        return any();
    }

	reader::iterator block_read(reader& rd, const range<reader::iterator>& buf)
	{
        reader::iterator first = buf.begin();
		while(first != buf.end())
		{
			if (!rd.readable())
				break;

			first = rd.read(make_range(first, buf.end()));
		}

		return first;
	}

	writer::const_iterator block_write(writer& wr, const range<writer::const_iterator>& buf)
	{
		writer::const_iterator first = buf.begin();
		while(first != buf.end())
		{
			range<writer::const_iterator> r = make_range(first, buf.end());
			first = wr.write(r);
		}
		return first;
	}

	long_size_t copy_data(reader& rd, writer& wr, long_size_t max_size /* = ~0 */)
	{
		aio::buffer<byte> buf;
		long_size_t nsize = 0;
		buf.resize(16 * 1024);		//16k bytes
		while (rd.readable() && wr.writable() && max_size > 0)
		{
			if (max_size < buf.size())
				buf.resize((size_t)max_size);
			reader::iterator pos = rd.read(to_range(buf));
			writer::const_iterator wr_pos = block_write(wr, aio::make_range(buf.begin(), pos));
			long_size_t wrsize = wr_pos - buf.begin();
			nsize += wrsize;
			max_size -= wrsize;
			if (wr_pos != buf.end())
				break;
		}
		return nsize;
	}

	long_size_t copy_archive(iarchive& from, iarchive& to, long_size_t max_size /* = ~0 */)
	{
		aio::archive::reader* rd = from.query_reader();
		aio::archive::writer* wr = to.query_writer();
		AIO_PRE_CONDITION(rd && wr);

		aio::archive::random* rnd = from.query_random();
		if (rnd) rnd->seek(0);
		rnd = to.query_random();
		if (rnd) rnd->seek(0);

		long_size_t nsize = copy_data(*rd, *wr, max_size);

		wr->truncate(nsize);
		return nsize;
	}

	void default_archive_deletor(iarchive* p)
	{
		if (p)
		{
			ideletor* dtor = p->query_deletor();
			AIO_PRE_CONDITION(dtor != 0);
			dtor->destroy();
		}
	}
}}

