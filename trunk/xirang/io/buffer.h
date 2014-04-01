#ifndef AIO_COMMON_IO_BUFFER_H_
#define AIO_COMMON_IO_BUFFER_H_
#include <xirang/io/locals11n.h>
#include <xirang/io/exchs11n.h>
#include <xirang/buffer.h>

namespace xirang{ namespace io{ namespace local{

	template<typename Ar, typename T, typename =
		typename std::enable_if<s11n::is_deserializer<Ar>::value>::type>
	Ar& save(Ar& ar, const buffer<T>& buf)
	{
		save(ar, buf.size());
		if (!buf.empty())
		{
			const byte* first = reinterpret_cast<const byte*>(buf.data());
			const byte* last = first + sizeof(T) * buf.size();
			get_interface<xirang::io::writer>(ar.get()).write(make_range(first, last));
		}
		return ar;
	}


	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_serializer<Ar>::value>::type>
	Ar& load(Ar& ar, buffer<T>& buf)
	{
		size_t size = load<size_t>(ar);
		buf.resize(size);
		if (size > 0)
		{
			byte* first = reinterpret_cast<byte*>(buf.data());
			byte* last = first + sizeof(T) * size;
			get_interface<xirang::io::reader>(ar.get()).read(make_range(first, last));
		}
		return ar;
	}

}}}
namespace xirang{namespace io{ namespace exchange{
	template<typename Ar, typename =
		typename std::enable_if< s11n::is_deserializer<Ar>::value>::type>
	Ar& load(Ar& ar, buffer<byte>& buf)
	{
		size_t size = exchange_cast<size_t>(load<uint32_t>(ar));

		buffer<byte> rbuf;
		rbuf.resize(size);
		block_read(ar.get(), make_range(rbuf.begin(), rbuf.end()));
		rbuf.swap(buf);

		return ar;
	}

	template<typename Ar, typename =
		typename std::enable_if< s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const buffer<byte>& buf)
	{
		uint32_t size = exchange_cast<uint32_t>(buf.size());
		save(ar, size);
		if (!buf.empty())
			block_write(ar.get(), make_range(buf.begin(), buf.end()));
		return ar;
	}

	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_deserializer<Ar>::value>::type>
	Ar& load(Ar& ar, buffer<T>& buf)
	{
		size_t size = exchange_cast<size_t>(load<uint32_t>(ar));

		typedef typename exchange_type_of<T>::type U;
		buffer<T> rbuf;
		rbuf.reserve(size);
		for (auto i = size; i > 0; --i){
			rbuf.push_back(exchange_cast<T>(load<U>(ar)));
		}
		buf.swap(buf);

		return ar;
	}

	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const buffer<T>& buf)
	{
		uint32_t size = exchange_cast<uint32_t>(buf.size());
		save(ar, local2ex_f(size));
		if (!buf.empty())
		{
			typedef typename exchange_type_of<T>::type U;
			for (auto i : buf)
				ar & exchange_cast<U>(i);
		}
		return ar;
	}
}}}
#endif //end AIO_COMMON_IO_BUFFER_H_
