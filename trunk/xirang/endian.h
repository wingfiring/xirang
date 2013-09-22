#ifndef AIO_COMMON_ENDIAN_H
#define AIO_COMMON_ENDIAN_H

#include <aio/common/config.h>
#include <endian.h>
namespace aio{
	struct little_endian_tag{};
	struct big_endian_tag{};
	struct pdp_endian_tag{};
	typedef little_endian_tag exchange_endian_tag;

# if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define AIO_LITTLE_ENDIAN
	typedef little_endian_tag local_endian_tag;
# elif (__BYTE_ORDER == __BIG_ENDIAN)
#  define AIO_BIG_ENDIAN
	typedef big_endian_tag local_endian_tag;
# elif (__BYTE_ORDER == __PDP_ENDIAN)
#  define AIO_PDP_ENDIAN
	typedef pdp_endian_tag local_endian_tag;
# else
#  error Unknown machine endianness detected.
# endif
# define AIO_BYTE_ORDER __BYTE_ORDER

}


#endif //end AIO_COMMON_ENDIAN_H
