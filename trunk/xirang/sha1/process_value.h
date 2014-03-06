#ifndef XIRANG_SHA1_PROCESS_VALUE_H
#define XIRANG_SHA1_PROCESS_VALUE_H

#include <xirang/sha1.h>
#include <xirang/io/exchs11n.h>

#include <xirang/buffer.h>
#include <xirang/string.h>
#include <xirang/versiontype.h>

namespace xirang{
	template<typename T, typename = typename std::enable_if<std::is_scalar<T>::value, void>::type>
	inline void process_value(sha1& sha, T t){
		byte const * p = (byte const*)&t;
		sha.write(make_range(p, p + sizeof(t)));
	}
}
#endif //end XIRANG_SHA1_PROCESS_VALUE_H
