#ifndef AIO_COMMON_IO_LOCAL_S11N_H
#define AIO_COMMON_IO_LOCAL_S11N_H
#include <xirang/io/s11nbase.h>

namespace aio{ namespace io{ namespace local{

	template<typename Ar> struct serializer : public s11n::serializer_base<Ar>{
		explicit serializer(Ar& ar) : s11n::serializer_base<Ar>(ar){};
	};
	template<typename Ar> struct deserializer : public s11n::deserializer_base<Ar>{
		explicit deserializer(Ar& ar) : s11n::deserializer_base<Ar>(ar){};
	};

	template<typename Ar> serializer<Ar> as_sink(Ar& ar){ return serializer<Ar>(ar);}
	template<typename Ar> deserializer<Ar> as_source(Ar& ar){ return deserializer<Ar>(ar);}

	template<typename Ar, typename T, 
		typename = typename std::enable_if<std::is_scalar<T>::value && 
			s11n::is_serializer<Ar>::value>::type>
		Ar save(Ar wt, const T& t)
		{
			const byte* first = reinterpret_cast<const byte*>(&t);
			const byte* last = reinterpret_cast<const byte*>(&t + 1);

			io::block_write(get_interface<io::writer>(wt.get()), make_range(first, last));

			return wt;
		}

	template<typename T, typename Ar,
		typename = typename std::enable_if<std::is_scalar<T>::value && 
			s11n::is_deserializer<Ar>::value>::type>
		Ar load(Ar rd, T& t)
		{
			byte* first = reinterpret_cast<byte*>(&t);
			byte* last = reinterpret_cast<byte*>(&t + 1);

			if (!io::block_read(get_interface<io::reader>(rd.get()), make_range(first, last)).empty() )
				AIO_THROW(io::read_failed);

			return rd;
		}

}}}

#endif //end AIO_COMMON_IO_LOCAL_S11N_H

