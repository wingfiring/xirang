#ifndef AIO_COMMON_ARCHIVE_FILE_ARCHIVE_H
#define AIO_COMMON_ARCHIVE_FILE_ARCHIVE_H

#include <aio/common/iarchive.h>
#include <aio/common/memory.h>

namespace aio{ namespace archive
{
	struct file_archive_imp;

	struct file_read_archive : archiveT<reader, random >
	{
		typedef reader::iterator iterator;

		explicit file_read_archive(const string& path);
		~file_read_archive();

		virtual iterator read(const range<iterator>& buf);
		virtual bool readable() const;
		virtual const_view view_rd(ext_heap::handle h) const;
		virtual bool viewable() const;

		virtual long_size_t offset() const;
		virtual long_size_t size() const;
		virtual long_size_t seek(long_size_t offset);

    private:
		file_archive_imp * m_imp;;
	};

	AIO_EXCEPTION_TYPE(archive_create_file_failed);
	AIO_EXCEPTION_TYPE(archive_open_file_failed);
	AIO_EXCEPTION_TYPE(archive_stat_file_failed);
	AIO_EXCEPTION_TYPE(archive_append_failed);

	struct file_write_archive : archiveT<writer, random >
	{
		typedef writer::const_iterator const_iterator;
		typedef writer::const_iterator iterator;

		explicit file_write_archive(const string& path,  int of);
		~file_write_archive();

		virtual const_iterator write(const range<const_iterator>& r);
		virtual long_size_t truncate(long_size_t size);
		virtual bool writable() const;
		virtual void sync() ;
		virtual view view_wr(ext_heap::handle h);
		virtual bool viewable() const;

		virtual long_size_t offset() const;
		virtual long_size_t size() const;
		virtual long_size_t seek(long_size_t offset);

    private:
		file_archive_imp * m_imp;;
	};

	struct file_read_write_archive : archiveT<reader, writer, random >
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;

		explicit file_read_write_archive(const string& path, int of);
		~file_read_write_archive();

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

	private:
		file_archive_imp * m_imp;;
	};

}
namespace io{
	struct file_archive_imp;

	struct file_read_archive // <reader, random >
	{
		typedef reader::iterator iterator;

		explicit file_read_archive(const string& path);
		~file_read_archive();

		range<iterator> read(const range<iterator>& buf);
		bool readable() const;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<read_view> view_rd(ext_heap::handle h) const;
    private:
		file_archive_imp * m_imp;;
	};

	AIO_EXCEPTION_TYPE(archive_create_file_failed);
	AIO_EXCEPTION_TYPE(archive_open_file_failed);
	AIO_EXCEPTION_TYPE(archive_stat_file_failed);
	AIO_EXCEPTION_TYPE(archive_append_failed);

	struct file_write_archive // <writer, random >
	{
		typedef writer::const_iterator const_iterator;
		typedef writer::const_iterator iterator;

		explicit file_write_archive(const string& path,  int of);
		~file_write_archive();

		range<const_iterator> write(const range<const_iterator>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<write_view> view_wr(ext_heap::handle h);
    private:
		file_archive_imp * m_imp;;
	};

	struct file_read_write_archive // <reader, writer, random >
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;

		explicit file_read_write_archive(const string& path, int of);
		~file_read_write_archive();

		range<iterator> read(const range<iterator>& buf);
		bool readable() const;

		range<const_iterator> write(const range<const_iterator>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<read_view> view_rd(ext_heap::handle h) const;
		unique_ptr<write_view> view_wr(ext_heap::handle h);
	private:
		file_archive_imp * m_imp;;
	};


}}

#endif //end AIO_COMMON_ARCHIVE_FILE_ARCHIVE_H

