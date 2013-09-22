#ifndef AIO_XIRANG_SERIALIZE_H_
#define AIO_XIRANG_SERIALIZE_H_

#include <xirang/io.h>
#include <xirang/io/s11n.h>
#include <xirang/object.h>
#include <xirang/binder.h>

//TODO: recheck
namespace xirang{ namespace type{ namespace io{ namespace exchange{

	// default imp of type method
	template<typename T> struct serializer{
		static io::writer& apply(io::writer& wt, ConstCommonObject obj){
			return io::exchange::as_sink(wt) & uncheckBind<T>(obj);
		}
	};

	template<typename T> struct deserializer{
		static io::reader& apply(io::reader& rd, CommonObject obj, heap& inner, ext_heap& ext){
			return io::exchange::as_source(rd) & uncheckBind<T>(obj);
		}
	};
}}}}

#endif //end AIO_XIRANG_SERIALIZE_H_

