#ifndef AIO_COMMON_ARCHIVE_EXT_HEAP_ARCHIVE_H
#define AIO_COMMON_ARCHIVE_EXT_HEAP_ARCHIVE_H

#include <aio/common/iarchive.h>
#include <aio/common/memory.h>

namespace aio{ namespace archive
{
	struct ext_heap_archive : archiveT<reader, writer, random>
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;
		ext_heap_archive(ext_heap& eh, ext_heap::handle h);
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

        ext_heap::handle get_handle() const;
	private:
		long_size_t m_pos;
		ext_heap& m_heap;
	};

}
}

#endif //end AIO_COMMON_ARCHIVE_EXT_HEAP_ARCHIVE_H