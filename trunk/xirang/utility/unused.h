#ifndef XIRANG_UTILITY_UNUSED_H
#define XIRANG_UTILITY_UNUSED_H

#include <xirang/config.h>
namespace xirang{
	//use to surppress unused variable warning.
	template<typename T>
	void unused(const T& ) {};
}


#endif //end XIRANG_UTILITY_UNUSED_H

