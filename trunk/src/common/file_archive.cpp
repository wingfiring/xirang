#include <aio/common/archive/file_archive.h>
#include <aio/common/to_string.h>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <boost/numeric/conversion/cast.hpp>
#include <aio/common/string_algo/utf8.h>
#include <aio/common/fsutility.h>

#ifdef MSVC_COMPILER_
#include <sys/stat.h>
#endif

namespace aio{ namespace io{

	using namespace boost::interprocess;
	namespace bi = boost::interprocess;
	using boost::numeric_cast;

	struct read_file_view : read_view{
			read_file_view(file_mapping& file, bi::mode_t mode, offset_t offset, std::size_t size)
				: m_region(file, mode, offset, size), m_offset(offset)
			{}
			virtual range<const byte*> address() const{ 
				const byte* first = reinterpret_cast<const byte*>(m_region.get_address());
				return range<const byte*>(first, first + m_region.get_size());
			}
		private:
			mapped_region m_region;
			offset_t m_offset;
	};
	struct write_file_view : write_view{
			write_file_view(file_mapping& file, bi::mode_t mode, offset_t offset, std::size_t size)
				: m_region(file, mode, offset, size), m_offset(offset)
			{}
			virtual range<byte*> address() const{ 
				byte* first = reinterpret_cast<byte*>(m_region.get_address());
				return range<byte*>(first, first + m_region.get_size());
			}
		private:
			mapped_region m_region;
			offset_t m_offset;
	};

	struct file_imp
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;

		file_imp(const string& path, int of, bi::mode_t mode)
			: m_path(path), m_pos(0), m_mode(mode), m_file_size(0), m_flag(of)
		{
			switch (of & of_low_mask)
			{
				case of_open:
					if (!exists_())
						AIO_THROW(archive_open_file_failed)(m_path.c_str());

					break;
				case of_create:
					if (exists_())
						AIO_THROW(archive_create_file_failed)(m_path.c_str());
					create_();

					break;
				case of_create_or_open:
					if (!exists_())
						create_();
					break;
				default:
					AIO_PRE_CONDITION(false);
			}

			try
			{
				m_file_size = get_file_size_();
#ifdef WIN32
                aio::wstring wpath = utf8::decode_string(fs::to_native_path(m_path));
				m_file = file_mapping(wpath.c_str(), mode);
#else
                m_file = file_mapping(m_path.c_str(), mode);
#endif
			}
			catch(...)
			{
				AIO_THROW(archive_open_file_failed)(m_path.c_str());
			}
		}
        ~file_imp()
        {
            if (m_flag & of_remove_on_close)
            {
                m_file.remove(m_path.c_str());
            }
            else
            {
                sync();
            }
        }
		range<iterator> read(const range<iterator>& buf)
		{
			AIO_PRE_CONDITION(m_pos <= m_file_size);

			long_size_t buf_size = buf.size();
			iterator itr = buf.begin();
			if (buf_size > 0 && readable() )
			{
				long_size_t view_size = std::min(buf_size, m_file_size - m_pos);
				mapped_region reg(m_file, m_mode, numeric_cast<std::size_t>(m_pos), numeric_cast<std::size_t>(view_size));

				const byte* psrc = (const byte*)reg.get_address();
				std::copy(psrc, psrc + view_size, itr);

				itr += view_size;
				m_pos += view_size;
			}
			return range<iterator>(itr, buf.end());
		}

		bool readable() const { return m_pos < m_file_size; }

		range<const_iterator> write(const range<const_iterator>& r)
		{
			long_size_t buf_size = r.size();
			if (buf_size > 0)
			{
				long_size_t new_pos = m_pos + buf_size;
				if (new_pos > m_file_size)
				{
					truncate(new_pos);
				}
				mapped_region reg(m_file, m_mode, numeric_cast<std::size_t>(m_pos), numeric_cast<size_t>(buf_size));
				byte* pdest = (byte*)reg.get_address();
				std::copy(r.begin(), r.end(), pdest);
				m_pos = new_pos;
			}
			return range<const_iterator>(r.end(), r.end());
		}

		long_size_t truncate(long_size_t nsize)
		{
#ifndef MSVC_COMPILER_
			::truncate(m_path.c_str(), nsize);
#else
            SetFilePointer(m_file.get_mapping_handle().handle, (LONG)nsize, 0, FILE_BEGIN);
            SetEndOfFile(m_file.get_mapping_handle().handle);

            //mapped_region reg(m_file, m_mode, 0, numeric_cast<size_t>(nsize));
#endif
			long_size_t fsize = get_file_size_();
			if (fsize != nsize)
				AIO_THROW(archive_append_failed)(to_string(nsize).c_str());
			m_file_size = nsize;
			if (m_pos > m_file_size)
				m_pos = m_file_size;

			return m_file_size;
		}
		bool writable() const { return true;}
		void sync() 
		{ 
			//TODO: imp 
		}
		unique_ptr<write_view> view_wr(ext_heap::handle h)
		{
			AIO_PRE_CONDITION(h.valid());
			if (m_file_size < numeric_cast<long_size_t>(h.end))
				truncate(h.end);

			return unique_ptr<write_view>(new write_file_view(m_file, m_mode, h.begin, numeric_cast<std::size_t>(h.size())));
		}

		unique_ptr<read_view> view_rd(ext_heap::handle h)
		{
			AIO_PRE_CONDITION(h.valid());

			if (m_file_size < numeric_cast<long_size_t>(h.end) )
				h.end = m_file_size;
			AIO_PRE_CONDITION(h.valid());

			return unique_ptr<read_view>(new read_file_view(m_file, m_mode, h.begin, numeric_cast<std::size_t>(h.size())));
		}

		long_size_t offset() const { return m_pos; }
		long_size_t size() const { return m_file_size;}
		long_size_t seek(long_size_t offset) 
		{
			m_pos = offset;
			return m_pos;
		}

        private:
		bool exists_()
		{
			
#ifndef MSVC_COMPILER_
            struct stat st;
			int ret = stat(m_path.c_str(), &st) ;
#else
            struct _stat st;
            aio::wstring wpath = utf8::decode_string(m_path);
			int ret = _wstat(wpath.c_str(), &st) ;
#endif
			return ret == 0;
		}

		void create_()
		{
#ifdef WIN32
            aio::wstring wpath = utf8::decode_string(m_path);
			FILE* fp = _wfopen(wpath.c_str(), L"wb");
#else
            FILE* fp = fopen(m_path.c_str(),"wb");
#endif
			if (fp == 0)
				AIO_THROW(archive_create_file_failed)(m_path.c_str());

			fclose(fp);
		}

		long_size_t get_file_size_()
		{
			
#ifndef MSVC_COMPILER_
            struct stat st;
			if (stat(m_path.c_str(), &st) != 0)
#else
            struct _stat st;
            aio::wstring wpath = utf8::decode_string(m_path);
			if (_wstat(wpath.c_str(), &st))
#endif			
				AIO_THROW(archive_stat_file_failed)(m_path.c_str());
			return st.st_size;
		}


		string m_path;
		long_size_t m_pos;
		bi::mode_t m_mode;
		long_size_t m_file_size;
		file_mapping m_file;
        int m_flag;
	};

	/////////////////////////////////////////////////
	file_reader::file_reader(const string& path)
		: m_imp(new file_imp(path, of_open, read_only))
	{
	}
	file_reader::~file_reader() { delete m_imp;}

	range<file_reader::iterator> file_reader::read(const range<file_reader::iterator>& buf)
	{
		return m_imp->read(buf);
	}
	bool file_reader::readable() const { return m_imp->readable();}
	unique_ptr<read_view> file_reader::view_rd(ext_heap::handle h) const { return m_imp->view_rd(h);}

	long_size_t file_reader::offset() const { return m_imp->offset();}
	long_size_t file_reader::size() const	{ return m_imp->size();}
	long_size_t file_reader::seek(long_size_t offset) { 
		if(offset > size()) offset = size();
		return m_imp->seek(offset);
	}

	/////////////////////////////////////////////////

	file_writer::file_writer(const string& path,  int of)
		: m_imp(new file_imp(path, of, read_write))
	{}

	file_writer::~file_writer() 	{ delete m_imp;}

	range<file_writer::const_iterator> file_writer::write(
			const range<file_writer::const_iterator>& r)
	{ return m_imp->write(r);}
	long_size_t file_writer::truncate(long_size_t size)	{ return m_imp->truncate(size);}
	bool file_writer::writable() const	{ return m_imp->writable(); }
	void file_writer::sync() 	{ m_imp->sync(); }
	unique_ptr<write_view> file_writer::view_wr(ext_heap::handle h){ return m_imp->view_wr(h);}

	long_size_t file_writer::offset() const	{ return m_imp->offset(); }
	long_size_t file_writer::size() const		{ return m_imp->size(); }
	long_size_t file_writer::seek(long_size_t offset) { return m_imp->seek(offset); }

	/////////////////////////////////////////////////

	file::file(const string& path, int of)
		: m_imp(new file_imp(path, of, read_write))
	{}
	file::~file()	{ delete m_imp;}

	range<file::iterator> file::read(
			const range<file::iterator>& buf)
	{	return m_imp->read(buf);	}

	bool file::readable() const	{ return m_imp->readable(); }
	unique_ptr<read_view> file::view_rd(ext_heap::handle h) const { return m_imp->view_rd(h);}
	range<file::const_iterator> file::write(
			const range<file::const_iterator>& r)
	{ return m_imp->write(r); }

	long_size_t file::truncate(long_size_t size)	{ return m_imp->truncate(size);}
	bool file::writable() const	{ return m_imp->writable(); }
	void file::sync() { m_imp->sync();}
	unique_ptr<write_view> file::view_wr(ext_heap::handle h){ return m_imp->view_wr(h);}

	long_size_t file::offset() const	{ return m_imp->offset(); }
	long_size_t file::size() const	{ return m_imp->size();}
	long_size_t file::seek(long_size_t offset)	{ return m_imp->seek(offset);}
} }

