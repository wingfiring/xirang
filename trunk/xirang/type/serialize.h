#ifndef AIO_XIRANG_SERIALIZE_H_
#define AIO_XIRANG_SERIALIZE_H_

#include <xirang/io.h>
#include <xirang/io/s11n.h>
#include <xirang/type/object.h>
#include <xirang/type/binder.h>

//TODO: recheck
namespace xirang{ namespace type{ namespace io{ namespace exchange{

	// default imp of type method
	template<typename T> struct serializer{
		static xirang::io::writer& apply(xirang::io::writer& wt, ConstCommonObject obj){
			return xirang::io::exchange::as_sink(wt) & uncheckBind<T>(obj);
		}
	};

	template<typename T> struct deserializer{
		static xirang::io::reader& apply(xirang::io::reader& rd, CommonObject obj, heap& inner, ext_heap& ext){
			return xirang::io::exchange::as_source(rd) & uncheckBind<T>(obj);
		}
	};
}}}}

#endif //end AIO_XIRANG_SERIALIZE_H_

