#include <aio/common/iarchive.h>
namespace aio{ namespace io{

	reader::~reader() {}
	writer::~writer() {}
	sequence::~sequence() {}
	forward::~forward() {}
	random::~random() {}
	read_view::~read_view(){}
	write_view::~write_view(){}
	read_map::~read_map(){}
	write_map::~write_map(){}

	range<reader::iterator> block_read(reader& rd, const range<reader::iterator>& buf)
	{
		auto reset = buf;
		while(!reset.empty() && rd.readable())
			reset = rd.read(reset);
		return reset;
	}
	range<writer::iterator> block_write(writer& wr, const range<writer::iterator>& buf){
		auto reset = buf;
		while(!reset.empty() && wr.writable())
			reset = wr.write(reset);
		return reset;
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
			auto rd_pos = rd.read(to_range(buf));
			auto wr_pos = block_write(wr, aio::make_range(buf.begin(), rd_pos.begin()));
			long_size_t wrsize = wr_pos.begin() - buf.begin();
			nsize += wrsize;
			max_size -= wrsize;
			if (!wr_pos.empty())
				break;
		}
		return nsize;
	}

}}

