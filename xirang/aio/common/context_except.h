#ifndef AIO_CONTEXT_EXCEPTION_H
#define AIO_CONTEXT_EXCEPTION_H
#include <aio/common/config.h>
#include <aio/common/macro_helper.h>
#include <aio/common/utf8char.h>

#include <aio/common/string.h>
//STL
//#include <string>
#include <aio/common/as_string.h>

#include <aio/common/config/abi_prefix.h>
namespace aio
{
	//used to collect exception point runtime information
	// @param base usually, user just need to define a empty exception class
	// with or without base class. all aio exception class should derived from ::aio::exception
	template<typename Base>
	class context_exception : public Base
	{
	public:
		/// just for nothow compatible
		virtual ~context_exception() AIO_COMPATIBLE_NOTHROW() {}

		/// append runtime info, char type (char, string ect) should use as<char> method
		/// @param t 
		context_exception& operator()(const const_range_string& value)
		{
			this->m_msg += value;
			return *this;
		}

		/// append runtime info, char type (char, string ect) should use as<char> method
		/// @param t 
		context_exception& operator()(const char_utf8* value)
		{
			if (value != 0)
				this->m_msg += value;
			return *this;
		}

		/// @return exception information, return as  char string
		virtual const char_utf8* what() const  AIO_COMPATIBLE_NOTHROW() {
			return this->m_msg.c_str();
		}

		virtual context_exception& append(const char_utf8* info) {
			return (*this)(info);
		}
	private:
		basic_string_builder<char_utf8> m_msg;
	};
}

#define AIO_THROW(ex) throw ::aio::context_exception<ex>()(__FILE__)("(")(aio::as_string<aio::char_utf8>(__LINE__).c_str())(")")

#include <aio/common/config/abi_suffix.h>
#endif //end AIO_CONTEXT_EXCEPTION_H
