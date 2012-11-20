//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_EXCEPTION_H
#define AIO_EXCEPTION_H
#include <aio/common/config.h>
#include <aio/common/macro_helper.h>
#include <aio/common/utf8char.h>

#ifdef AIO_DERIVED_FROM_STD_EXCEPTION
#include <exception>
#endif


#include <aio/common/config/abi_prefix.h>
namespace aio
{
	struct exception
#ifdef AIO_DERIVED_FROM_STD_EXCEPTION
		:public std::exception
#endif
	{
		virtual const char_utf8* what() const AIO_COMPATIBLE_NOTHROW()  = 0;
		virtual exception& append(const char_utf8* info)  = 0;
		virtual ~exception() AIO_COMPATIBLE_NOTHROW() {};
	};
}

#define AIO_EXCEPTION_TYPE_EX(type, base) \
	struct type : public base {\
	virtual ~type() AIO_COMPATIBLE_NOTHROW() {}\
	}

#define AIO_EXCEPTION_TYPE(type) \
	AIO_EXCEPTION_TYPE_EX(type, ::aio::exception)

#include <aio/common/config/abi_suffix.h>
#endif //end AIO_EXCEPTION_H