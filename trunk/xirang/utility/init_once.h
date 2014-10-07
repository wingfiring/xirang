#ifndef XIRANG_UTILITY_INIT_ONCE_H
#define XIRANG_UTILITY_INIT_ONCE_H
#include <xirang/config.h>
namespace xirang{
	template<typename T>
	class init_once
	{
		static int refcount;
	public:
		init_once()
		{
			if (refcount++ == 0)
			  	T::init();
		}
		~init_once()
		{
			if (refcount == 0)
				T::uninit();
		}
	};

	template<typename T>
	int init_once<T>::refcount = 0;

#define AIO_INIT_ONCE(T)\
	namespace { \
	::xirang::init_once<T> g_init_once;}


}
#endif //end XIRANG_UTILITY_INIT_ONCE_H


