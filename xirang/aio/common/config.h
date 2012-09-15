//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_CONFIG_H
#define AIO_COMMON_CONFIG_H

#include <aio/common/config/config.h>
#include <aio/common/config/user.h>

#if defined (AIO_COMMON_DLL_EXPORT)
#	define AIO_COMM_API AIO_DLL_EXPORT
#elif defined (AIO_COMMON_DLL_IMPORT)
#	define AIO_COMM_API AIO_DLL_IMPORT
#else
#	define AIO_COMM_API 
#endif

template<typename T>
inline void unuse(const T&) {}

namespace aio{
	template<typename T> constexpr T const& const_max(T const& a, T const& b) {
		return a < b ? b : a;
	}
	template<typename T> constexpr T const& const_min(T const& a, T const& b) {
		return a < b ? a : b;
	}


	template<typename Base> struct identity_base0 : Base{};
    template<typename Base> struct identity_base1 : Base{};
    template<typename Base> struct identity_base2 : Base{};
    template<typename Base> struct identity_base3 : Base{};
    template<typename Base> struct identity_base4 : Base{};
    template<typename Base> struct identity_base5 : Base{};

    struct null_type{};
}

#endif //end AIO_COMMON_CONFIG_H


