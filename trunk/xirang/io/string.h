#ifndef AIO_COMMON_ARCHIVE_STRING_H
#define AIO_COMMON_ARCHIVE_STRING_H

#include <xirang/io/locals11n.h>
#include <xirang/io/exchs11n.h>
#include <xirang/string.h>

namespace xirang{namespace io{ namespace local{
	template<typename Ar, typename T, typename =
		typename std::enable_if<s11n::is_deserializer<Ar>::value>::type>
	Ar& load(Ar& ar, basic_range_string<T>& buf)
	{
		typename basic_range_string<T>::pointer first, last;
		ar & first & last;
		buf = basic_range_string<T>(first, last);

		return ar;
	}

	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const basic_range_string<T>& buf)
	{
		return ar & buf.begin() & buf.end();
	}


	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_deserializer<Ar>::value>::type>
	Ar& load(Ar& ar, basic_string<T>& buf)
	{
		size_t size = load<size_t>(ar);
		if (size > 0)
		{
			buffer<byte> rbuf;
			rbuf.resize(size * sizeof(T));
			block_read(ar.get(), make_range(rbuf.begin(), rbuf.end()));

			T* first = (T*)rbuf.begin();
			T* last = first + size;
			buf = make_range(first, last);
		}

		return ar;
	}

	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const basic_string<T>& buf)
	{
		save(ar, buf.size());
		if (!buf.empty())
		{
			const byte* first = reinterpret_cast<const byte*>(buf.data());
			const byte* last = first + sizeof(T) * buf.size();
			block_write(ar.get(), make_range(first, last));
		}
		return ar;
	}

}}}

namespace xirang{namespace io{ namespace exchange{

	template<typename Ar, typename =
		typename std::enable_if< s11n::is_deserializer<Ar>::value>::type>
	Ar& load(Ar& ar, basic_string<char>& buf)
	{
		size_t size = exchange_cast<size_t>(load<uint32_t>(ar));
		if (size > 0)
		{
			buffer<byte> rbuf;
			rbuf.resize(size);
			block_read(ar.get(), make_range(rbuf.begin(), rbuf.end()));

			char* first = (char*)rbuf.begin();
			char* last = first + size;
			buf = make_range(first, last);
		}
		else
			buf = basic_string<char>();

		return ar;
	}

	template<typename Ar, typename =
		typename std::enable_if< s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const basic_string<char>& str)
	{
		uint32_t size = exchange_cast<uint32_t>(str.size());
		save(ar, size);
		if (!str.empty())
		{
			const byte* first = reinterpret_cast<const byte*>(&*str.begin());
			const byte* last = first + str.size();
			block_write(ar.get(), make_range(first, last));
		}
		return ar;
	}
	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_deserializer<Ar>::value>::type>
	Ar& load(Ar& ar, basic_string<T>& str)
	{
		size_t size = exchange_cast<size_t>(load<uint32_t>(ar));
		if (size > 0)
		{
			typedef typename exchange_type_of<T>::type U;
			buffer<T> rbuf;
			rbuf.reserve(size);
			for (auto i = size; i > 0; --i){
				rbuf.push_back(exchange_cast<T>(load<U>(ar)));
			}
			str = make_range(rbuf.begin(), rbuf.end());
		}
		else
			str = basic_string<T>();

		return ar;
	}

	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const basic_string<T>& str)
	{
		uint32_t size = exchange_cast<uint32_t>(str.size());
		save(ar, size);
		if (!str.empty())
		{
			typedef typename exchange_type_of<T>::type U;
			for (auto i : str)
				ar & exchange_cast<U>(i);
		}
		return ar;
	}


}}}

#endif //end AIO_COMMON_ARCHIVE_STRING_H


