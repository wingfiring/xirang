//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_AS_STRING_H
#define AIO_AS_STRING_H
#include <aio/common/config.h>
#include <aio/common/macro_helper.h>
#include <aio/common/string_cast.h>

//STL
#include <string>

namespace aio {

/// use to convert and strore serials of data as string
template<typename CharT>
struct as_string
{
	///\ctor
	as_string(){}

	///\ctor template version
	template<typename T>
	explicit as_string(const T& value){
		(*this)(value);
	}

	///convert the value to string, then append to the current str.
	template<typename T>
	as_string& operator()(const T& value){
		str += string_cast<CharT>(value);
		return *this;
	}

	const CharT* c_str() const { return str.c_str();}

	///append result
	aio::basic_string_builder<CharT> str;
};

}

#endif //end AIO_AS_STRING_H

