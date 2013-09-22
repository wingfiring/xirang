#ifndef AIO_XIRANG_SERIALIZE_H_
#define AIO_XIRANG_SERIALIZE_H_

#include <xirang/io.h>
#include <xirang/io/s11n.h>
#include <xirang/object.h>
#include <xirang/binder.h>

namespace xirang{ namespace io{ namespace exchange{

	// default imp of type method
	template<typename T> struct serializer{
		static aio::io::writer& apply(aio::io::writer& wt, ConstCommonObject obj){
			return aio::io::exchange::as_sink(wt) & uncheckBind<T>(obj);
		}
	};

	template<typename T> struct deserializer{
		static aio::io::reader& apply(aio::io::reader& rd, CommonObject obj, heap& inner, ext_heap& ext){
			return aio::io::exchange::as_source(rd) & uncheckBind<T>(obj);
		}
	};
}}}

#endif //end AIO_XIRANG_SERIALIZE_H_

