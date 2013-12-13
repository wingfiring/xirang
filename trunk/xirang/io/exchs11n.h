#ifndef AIO_COMMON_IO_S11N_EXCHANGE_H_
#define AIO_COMMON_IO_S11N_EXCHANGE_H_
#include <xirang/io/s11nbase.h>
#include <xirang/byteorder.h>
#include <limits>

namespace xirang{ namespace io{ namespace exchange{

	template<typename Ar> struct serializer : public s11n::serializer_base<Ar>{
		explicit serializer(Ar& ar) : s11n::serializer_base<Ar>(ar){};
	};
	template<typename Ar> struct deserializer : public s11n::deserializer_base<Ar>{
		explicit deserializer(Ar& ar) : s11n::deserializer_base<Ar>(ar){};
	};
	template<typename Ar> serializer<Ar> as_sink(Ar& ar){ return serializer<Ar>(ar);}
	template<typename Ar> deserializer<Ar> as_source(Ar& ar){ return deserializer<Ar>(ar);}

	//TODO: map T to a exchangable type U
	template<typename T> struct exchange_type_of{ typedef T type;};

	template<> struct exchange_type_of<wchar_t>{ typedef std::int32_t type;};
	template<> struct exchange_type_of<char>{ typedef std::int8_t type;};

	AIO_EXCEPTION_TYPE(bad_exchange_cast);

	template<typename T, typename U> struct exchange_cast_imp{
		static U apply(T t){
			if (t < std::numeric_limits<U>::min()
					|| t > std::numeric_limits<U>::max())
				AIO_THROW(bad_exchange_cast);
			return U(t);
		}
	};
	template<typename T> struct exchange_cast_imp<T, T>{
		static T apply(T t){
			return t;
		}
	};

	template<typename U, typename T> U exchange_cast(T t){
		return exchange_cast_imp<T, U>::apply(t);
	}

	template<typename Ar, typename T, 
		typename = typename std::enable_if<std::is_scalar<T>::value && 
			s11n::is_serializer<Ar>::value>::type>
		Ar save(Ar wt, const T& v)
		{
			typedef typename exchange_type_of<T>::type U;
			U t = local2ex_f(exchange_cast<U>(v));

			const byte* first = reinterpret_cast<const byte*>(&t);
			const byte* last = reinterpret_cast<const byte*>(&t + 1);

			io::block_write(get_interface<io::writer>(wt.get()), make_range(first, last));

			return wt;
		}

	template<typename T, typename Ar,
		typename = typename std::enable_if<std::is_scalar<T>::value && 
			s11n::is_deserializer<Ar>::value>::type>
		Ar load(Ar rd, T& v)
		{
			typedef typename exchange_type_of<T>::type U;
			U t;
			byte* first = reinterpret_cast<byte*>(&t);
			byte* last = reinterpret_cast<byte*>(&t + 1);

			if (!io::block_read(get_interface<io::reader>(rd.get()), make_range(first, last)).empty() )
				AIO_THROW(io::read_exception);

			v = exchange_cast<T>(ex2local_f(t));

			return rd;
		}

}}}

#endif //end AIO_COMMON_IO_S11N_EXCHANGE_H_

