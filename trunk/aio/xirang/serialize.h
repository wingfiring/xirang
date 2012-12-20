#ifndef AIO_XIRANG_SERIALIZE_H_
#define AIO_XIRANG_SERIALIZE_H_

#include <aio/common/iarchive.h>
#include <aio/common/archive/string.h>
#include <aio/xirang/object.h>
#include <aio/xirang/binder.h>

namespace xirang{ namespace sio{
	using aio::io::reader;
	using aio::io::writer;
	using namespace aio::sio;
	
	//overide buffer
	using aio::buffer;

	extern uint32_t numeric_uint32_cast(size_t n);

	template<typename Ar, typename T> Ar& save(Ar& ar, const buffer<T>& buf)
	{
		uint32_t size = numeric_uint32_cast(buf.size());
		save(ar, size);
		if (!buf.empty())
		{
			const byte* first = reinterpret_cast<const byte*>(buf.data());
			const byte* last = first + sizeof(T) * buf.size();
			ar.write(make_range(first, last));
		}
		return ar;
	}

	template<typename Ar, typename T> Ar& load(Ar& ar, buffer<T>& buf)
	{
		uint32_t size = load<uint32_t>(ar);
		buf.resize(size);
		if (size > 0)
		{   
			byte* first = reinterpret_cast<byte*>(buf.data());
			byte* last = first + sizeof(T) * size;
			ar.read(make_range(first, last));
		}
		return ar;
	}

	//overide string
	using aio::basic_string;
	using aio::basic_string_builder;

	template<typename Archive, typename T, typename = 
		typename std::enable_if< std::is_convertible<Archive&, reader&>::value , Archive&>::type>
	Archive& load(Archive& ar, basic_string<T>& buf)
	{
        buf.clear();

		uint32_t size = load<uint32_t>(ar);
		if (size > 0)
		{
			buffer<byte> rbuf;
			rbuf.resize(size * sizeof(T));
			block_read(ar, make_range(rbuf.begin(), rbuf.end()));
			
			T* first = (T*)rbuf.begin();
			T* last = first + size;
			buf = make_range(first, last);
		}

		return ar;
	}

	template<typename Archive, typename T, typename =
		typename std::enable_if< std::is_convertible<Archive&, writer&>::value , Archive&>::type>
	Archive& save(Archive& ar, const basic_string<T>& buf)
	{
		uint32_t size = numeric_uint32_cast(buf.size());
		ar & size;
		if (!buf.empty())
		{
			const byte* first = reinterpret_cast<const byte*>(buf.data());
			const byte* last = first + sizeof(T) * buf.size();
			block_write(ar, make_range(first, last));
		}
		return ar;
	}


	// default imp of type method
	template<typename T> struct serializer{
		static aio::io::writer& apply(aio::io::writer& wt, ConstCommonObject obj){
			return wt & uncheckBind<T>(obj);
		}
	};

	template<typename T> struct deserializer{
		static aio::io::reader& apply(aio::io::reader& rd, CommonObject obj, heap& inner, ext_heap& ext){
			return rd & uncheckBind<T>(obj);
		}
	};
}


}
#endif //end AIO_XIRANG_SERIALIZE_H_

