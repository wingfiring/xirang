//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_STRING_CAST_H
#define AIO_STRING_CAST_H
#include <xirang/config.h>
#include <xirang/macro_helper.h>
#include <xirang/string_algo/utf8.h>
#include <xirang/range.h>

//STL
#include <cstring>	//for strlen
#include <sstream>	//for stringstream wstringstream

namespace aio { namespace private_ {

template<typename CharT>
struct string_converter;

template<>
struct string_converter<char>
{
	template<typename T>
	static string convert(const T& value)
	{
		return convert_(value);
	}

private:
	static string convert_(wchar_t value)
	{
		wchar_t arr[] = {value, 0};
		return convert_((const wchar_t*)arr);
	}

	static string convert_(const wchar_t* value)
	{
		if (value == 0) return literal("(null)");
		
		return utf8::encode_string(make_range(value, value + std::wcslen(value)));
	}	

	static string convert_(const char* value)
	{
		if (value == 0) return literal("(null)");
		return as_range_string(value);
	}

	template<std::size_t N>
	static string convert_(const wchar_t (&value)[N])
	{
		return convert_((const wchar_t*)value);
	}	

	/// append runtime info, char type (char, string ect) should use as<char> method
	/// @param value 
	template< typename T>
	static string convert_(const T& value)
	{
		std::stringstream sstr;
        sstr << value;
        return basic_range_string<const char_utf8>(sstr.str());
	}

	static string convert_(const wstring& value)
	{
		return utf8::encode_string(value);
	}
};

template<>
struct string_converter<wchar_t>
{
	template<typename T>
	static wstring convert(const T& value)
	{
		return convert_(value);
	}

private:
	static wstring convert_(char value)
	{
		return wstring_builder(1, wchar_t(value)).str();
	}

	static wstring convert_(const char* value)
	{
		if (value == 0) return literal(L"(null)");
		
		return utf8::decode_string(make_range(value, value + std::strlen(value)));
	}	

	static wstring convert_(const wchar_t* value)
	{
		if (value == 0) return literal(L"(null)");

		return as_range_string(value);
	}

	template<std::size_t N>
	static wstring convert_(const char (&value)[N])
	{
		return convert_((const char*)value);
	}

	/// append runtime info, char type (char, string ect) should use as<char> method
	/// @param value 
	template< typename T>
	static wstring convert_(const T& value)
	{
		std::wstringstream sstr;
        sstr <<value;
        return sstr.str();
	}

	static wstring convert_(const string& value)
	{
		return convert_(value.c_str());
	}
};
}	//namespace private_

template<typename CharT, typename T>
basic_string<CharT> string_cast(const T& value)
{
	return private_::string_converter<CharT>::convert(value);
}

/// use to convert and strore serials of data as string
template<typename CharT>
struct basic_to_string
{
	///\ctor
	basic_to_string(){}

	///\ctor template version
	template<typename T>
	explicit basic_to_string(const T& value){
		(*this)(value);
	}

	///convert the value to string, then append to the current str.
	template<typename T>
	basic_to_string& operator()(const T& value){
		str += string_cast<CharT>(value);
		return *this;
	}

	const CharT* c_str() const { return str.c_str();}

	///append result
	aio::basic_string_builder<CharT> str;
};

typedef basic_to_string<char>  to_string;
typedef basic_to_string<char>  to_wstring;

}

#endif //end AIO_STRING_CAST_H

