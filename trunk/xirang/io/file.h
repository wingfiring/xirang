#ifndef AIO_COMMON_ARCHIVE_FILE_ARCHIVE_H
#define AIO_COMMON_ARCHIVE_FILE_ARCHIVE_H

#include <xirang/io.h>
#include <xirang/memory.h>
#include <xirang/path.h>

namespace xirang{ namespace io{
	struct file_imp;

	struct file_reader // <reader, random >
	{
		typedef reader::iterator iterator;

		explicit file_reader(const file_path& path);
		~file_reader();

		range<iterator> read(const range<iterator>& buf);
		bool readable() const;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<read_view> view_rd(ext_heap::handle h) const;
    private:
		file_imp * m_imp;;
	};

	AIO_EXCEPTION_TYPE(archive_append_failed);

	struct file_writer // <writer, random >
	{
		typedef writer::const_iterator const_iterator;
		typedef writer::const_iterator iterator;

		explicit file_writer(const file_path& path,  int of);
		~file_writer();

		range<const_iterator> write(const range<const_iterator>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<write_view> view_wr(ext_heap::handle h);
    private:
		file_imp * m_imp;;
	};

	struct file // <reader, writer, random >
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;

		explicit file(const file_path& path, int of);
		~file();

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
		file_imp * m_imp;;
	};


}}

#endif //end AIO_COMMON_ARCHIVE_FILE_ARCHIVE_H

