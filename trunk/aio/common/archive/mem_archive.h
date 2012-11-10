#ifndef AIO_COMMON_ARCHIVE_MEM_ARCHIVE_H
#define AIO_COMMON_ARCHIVE_MEM_ARCHIVE_H

#include <aio/common/iarchive.h>

namespace aio{ namespace archive {
	struct buffer_in : archiveT<reader, random>
	{
		typedef reader::iterator iterator;
		explicit buffer_in(const buffer<byte>& buf);

		virtual iterator read(const range<iterator>& buf);
		virtual bool readable() const;
		virtual const_view view_rd(ext_heap::handle h) const;
		virtual bool viewable() const;

		virtual long_size_t offset() const;
		virtual long_size_t size() const;
		virtual long_size_t seek(long_size_t offset);

		const buffer<byte> & data();
	private:
		long_size_t m_pos;
		const buffer<byte>& m_data;

	};

	struct buffer_out : archiveT<writer, random>
	{
		typedef writer::const_iterator const_iterator;
		explicit buffer_out(buffer<byte>& buf);

		virtual const_iterator write(const range<const_iterator>& r);
		virtual long_size_t truncate(long_size_t size);
		virtual bool writable() const;
		virtual void sync() ;
		virtual view view_wr(ext_heap::handle h);
		virtual bool viewable() const;

		virtual long_size_t offset() const;
		virtual long_size_t size() const;
		virtual long_size_t seek(long_size_t offset);

		buffer<byte> & data();
	private:
		size_t m_pos;
		buffer<byte>& m_data;
	};

	struct buffer_io : archiveT<reader, writer, random>
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;

		explicit buffer_io(buffer<byte>& buf);
		virtual iterator read(const range<iterator>& buf);
		virtual bool readable() const;
		virtual const_view view_rd(ext_heap::handle h) const;
		virtual const_iterator write(const range<const_iterator>& r);
		virtual long_size_t truncate(long_size_t size);
		virtual bool writable() const;
		virtual void sync() ;
		virtual view view_wr(ext_heap::handle h);
		virtual bool viewable() const;

		virtual long_size_t offset() const;
		virtual long_size_t size() const;
		virtual long_size_t seek(long_size_t offset);

		buffer<byte> & data();
	private:
		size_t m_pos;
		buffer<byte>& m_data;
	};

	typedef buffer_in mem_read_archive;

	struct mem_write_archive : buffer_out
	{
		mem_write_archive();
	private:
		buffer<byte> m_data;
	};

	struct mem_read_write_archive : buffer_io
	{
		mem_read_write_archive();
		explicit mem_read_write_archive(const buffer<byte>& buf);
	private:
		buffer<byte> m_data;
	};
}
namespace io{

	struct buffer_rd_view : read_view
	{
		range<const byte*> address() const { return m_data;}

		buffer_rd_view() : m_data(){}
		explicit buffer_rd_view(range<const byte*> data) : m_data(data){}
		private:
		range<const byte*> m_data;
	};

	struct buffer_wr_view : write_view
	{
		range<byte*> address() const { return m_data;}

		buffer_wr_view() : m_data(){}
		explicit buffer_wr_view(range<byte*> data) : m_data(data){}
		private:
		range<byte*> m_data;
	};

	struct buffer_in	// reader, random, const_view
	{
		typedef byte* iterator;
		explicit buffer_in(const buffer<byte>& buf);

		range<iterator> read(const range<byte*>& buf);
		bool readable() const;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<read_view> view_rd(ext_heap::handle h) const;

		const buffer<byte> & data();
	private:
		long_size_t m_pos;
		const buffer<byte>& m_data;

	};

	struct buffer_out	//  writer, random, view
	{
		typedef const byte* const_iterator;
		explicit buffer_out(buffer<byte>& buf);

		range<const byte*> write(const range<const byte*>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<write_view> view_wr(ext_heap::handle h);
		buffer<byte> & data();
	private:
		size_t m_pos;
		buffer<byte>& m_data;
	};

	struct buffer_io // reader, writer, random, const_view, view
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;

		explicit buffer_io(buffer<byte>& buf);
		range<iterator> read(const range<byte*>& buf);
		bool readable() const;

		range<const byte*> write(const range<const byte*>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<read_view> view_rd(ext_heap::handle h) const;
		unique_ptr<write_view> view_wr(ext_heap::handle h);

		buffer<byte> & data();
		private:
		size_t m_pos;
		buffer<byte>& m_data;
	};

	typedef buffer_in mem_read_archive;

	struct mem_write_archive : buffer_out
	{
		mem_write_archive();
		private:
		buffer<byte> m_data;
	};

	struct mem_read_write_archive : buffer_io
	{
		mem_read_write_archive();
		explicit mem_read_write_archive(const buffer<byte>& buf);
		private:
		buffer<byte> m_data;
	};
}

}

#endif //end AIO_COMMON_ARCHIVE_MEM_ARCHIVE_H

