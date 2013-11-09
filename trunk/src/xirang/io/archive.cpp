#include <xirang/io.h>
namespace xirang{ namespace io{

	const long_size_t K_ViewSize = 64 * 1024;

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
		buffer<byte> buf;
		long_size_t nsize = 0;
		buf.resize(16 * 1024);		//16k bytes
		while (rd.readable() && wr.writable() && max_size > 0)
		{
			if (max_size < buf.size())
				buf.resize((size_t)max_size);
			auto rd_pos = rd.read(to_range(buf));
			auto wr_pos = block_write(wr, make_range(buf.begin(), rd_pos.begin()));
			long_size_t wrsize = wr_pos.begin() - buf.begin();
			nsize += wrsize;
			max_size -= wrsize;
			if (!wr_pos.empty())
				break;
		}
		return nsize;
	}

	long_size_t copy_data(reader& rd, write_map& wr, long_size_t max_size /*  = ~0 */){
		long_size_t nsize = 0;
		while (rd.readable() && nsize < max_size)
		{
			auto view = wr.view_wr(ext_heap::handle(nsize, nsize + K_ViewSize));
			auto buf = view.get<write_view>().address();
			if (buf.empty())
				break;
			auto res = rd.read(buf);
			nsize += buf.size() - res.size();
		}
		wr.truncate(nsize);
		return nsize;
	}
	long_size_t copy_data(read_map& rd, writer& wr, long_size_t max_size  /* = ~0 */){
		max_size = std::min(max_size, rd.size());

		long_size_t nsize = 0;
		while (wr.writable() && nsize < max_size)
		{
			auto view = rd.view_rd(ext_heap::handle(nsize, std::min(max_size, nsize + K_ViewSize)));
			auto buf = view.get<read_view>().address();
			if (buf.empty())
				break;
			auto res = wr.write(buf);
			nsize += buf.size() - res.size();
		}
		return nsize;
	}
	long_size_t copy_data(read_map& rd, write_map& wr, long_size_t max_size /*  = ~0*/ ){
		max_size = std::min(max_size, rd.size());

		long_size_t nsize = 0;
		while (nsize < max_size)
		{
			auto rview = rd.view_rd(ext_heap::handle(nsize, std::min(max_size, nsize + K_ViewSize)));
			auto wview = wr.view_wr(ext_heap::handle(nsize, std::min(max_size, nsize + K_ViewSize)));

			auto rbuf = rview.get<read_view>().address();
			auto wbuf = wview.get<write_view>().address();

			auto s = std::min(rbuf.size(), wbuf.size());
			std::copy(rbuf.begin(), rbuf.begin() + s, wbuf.begin());
			nsize += s;
		}
		return nsize;
	}


}}

