#ifndef AIO_COMMON_IO_BUFFER_H_
#define AIO_COMMON_IO_BUFFER_H_
#include <xirang/io/locals11n.h>
#include <xirang/io/exchs11n.h>
#include <xirang/buffer.h>

namespace aio{ namespace io{ namespace local{

	template<typename Ar, typename T, typename = 
		typename std::enable_if<s11n::is_deserializer<Ar>::value>::type>
	Ar save(Ar ar, const buffer<T>& buf)
	{
		save(ar, buf.size());
		if (!buf.empty())
		{
			const byte* first = reinterpret_cast<const byte*>(buf.data());
			const byte* last = first + sizeof(T) * buf.size();
			get_interface<aio::io::writer>(ar.get()).write(make_range(first, last));
		}
		return ar;
	}


	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_serializer<Ar>::value>::type>
	Ar load(Ar ar, buffer<T>& buf)
	{
		size_t size = load<size_t>(ar);
		buf.resize(size);
		if (size > 0)
		{   
			byte* first = reinterpret_cast<byte*>(buf.data());
			byte* last = first + sizeof(T) * size;
			get_interface<aio::io::reader>(ar.get()).read(make_range(first, last));
		}
		return ar;
	}

}}}
namespace aio{namespace io{ namespace exchange{
	template<typename Ar, typename T, typename = 
		typename std::enable_if< s11n::is_deserializer<Ar>::value>::type>
	Ar load(Ar ar, buffer<T>& buf)
	{
		size_t size = ex2local_f(load<uint32_t>(ar));
		if (size > 0)
		{
			buffer<byte> rbuf;
			rbuf.resize(size * sizeof(T));
			block_read(ar.get(), make_range(rbuf.begin(), rbuf.end()));
			
			T* first = (T*)rbuf.begin();
			T* last = first + size;
			std::transform(first, last, first, ex2local<T>());
			buf = make_range(first, last);
		}

		return ar;
	}

	template<typename Ar, typename T, typename =
		typename std::enable_if< s11n::is_serializer<Ar>::value>::type>
	Ar save(Ar ar, const buffer<T>& str)
	{
		uint32_t size = exchange_cast<uint32_t>(str.size());
		save(ar, local2ex_f(size));
		if (!str.empty())
		{
			buffer<byte> buf;
			buf.resize(size * sizeof(T));
			const byte* first = reinterpret_cast<const byte*>(buf.data());
			const byte* last = first + sizeof(T) * buf.size();
			std::transform(str.begin(), str.end(), first, local2ex<T>());
			block_write(ar.get(), make_range(first, last));
		}
		return ar;
	}

}}}

#endif //end AIO_COMMON_IO_BUFFER_H_
