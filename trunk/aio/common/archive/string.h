#ifndef AIO_COMMON_ARCHIVE_STRING_H
#define AIO_COMMON_ARCHIVE_STRING_H

#include <aio/common/iarchive.h>
#include <aio/common/string.h>

namespace aio{ namespace archive{

	template<typename Archive, typename T>
	typename std::enable_if< std::is_convertible<Archive&, reader&>::value , Archive&>::type
	operator &(Archive& ar, basic_range_string<T>& buf)
	{
		typename basic_range_string<T>::pointer first, last;
		ar & first & last;
		buf = basic_range_string<T>(first, last);

		return ar;
	}

	template<typename Archive, typename T>
	typename std::enable_if< std::is_convertible<Archive&, writer&>::value , Archive&>::type
	operator &(Archive& ar, const basic_range_string<T>& buf)
	{
		return ar & buf.begin() & buf.end();
	}


	template<typename Archive, typename T>
	typename std::enable_if< std::is_convertible<Archive&, reader&>::value , Archive&>::type
	operator &(Archive& ar, basic_string<T>& buf)
	{
        buf.clear();

		uint32_t size;
		ar & size;
		if (size > 0)
		{
			buffer<byte> rbuf;
			rbuf.resize(size);
			block_read(ar, make_range(rbuf.begin(), rbuf.end()));
			
			T* first = (T*)rbuf.begin();
			T* last = first + size;
			buf = make_range(first, last);
		}

		return ar;
	}

	template<typename Archive, typename T>
	typename std::enable_if< std::is_convertible<Archive&, writer&>::value , Archive&>::type
	operator &(Archive& ar, const basic_string<T>& buf)
	{
        uint32_t size =  (uint32_t)buf.size();
		ar & size;
		if (!buf.empty())
		{
			const byte* first = reinterpret_cast<const byte*>(buf.data());
			const byte* last = first + sizeof(T) * buf.size();
			block_write(ar, make_range(first, last));
		}
		return ar;
	}

}
}

#endif //end AIO_COMMON_ARCHIVE_STRING_H


