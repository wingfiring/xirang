#ifndef AIO_STRING_CAST_H
#define AIO_STRING_CAST_H
#include <aio/common/config.h>
#include <aio/common/macro_helper.h>

//BOOST
//#include <boost/lexical_cast.hpp>

//STL
#include <cstdlib>	//for std::mbstowcs, std::wcstombs
#include <string>
#include <cstring>
#include <cwchar>
#include <sstream>

namespace aio { namespace private_ {

template<typename CharT>
struct string_converter;

template<>
struct string_converter<char>
{
	template<typename T>
	static std::string convert(const T& value)
	{
		return convert_(value);
	}

private:
	static std::string convert_(wchar_t value)
	{
		wchar_t arr[] = {value, 0};
		return convert_((const wchar_t*)arr);
	}

	static std::string convert_(const wchar_t* value)
	{
		if (value == 0)
			return convert_((void*)0);
		
		std::string result;
		std::size_t length = std::wcslen(value);
		result.resize(length * sizeof (wchar_t));
		std::size_t size = std::wcstombs(const_cast<char*>(result.data()), value, length);
		if (size == std::size_t(-1))
			result.clear();
		else
			result.resize(size);
		return result;
	}	

	static std::string convert_(const char* value)
	{
		if (value == 0)
			return convert((void*)0);
	
		return value;
	}

	template<std::size_t N>
	static std::string convert_(const wchar_t (&value)[N])
	{
		return convert_((const wchar_t*)value);
	}	

	/// append runtime info, char type (char, string ect) should use as<char> method
	/// @param value 
	template< typename T>
	static std::string convert_(const T& value)
	{
        std::stringstream sstr;
        sstr <<value;
        return sstr.str();
		//return boost::lexical_cast<std::string>(value);
	}

	static std::string convert_(const std::wstring& value)
	{
		return convert_(value.c_str());
	}
};

template<>
struct string_converter<wchar_t>
{
	template<typename T>
	static std::wstring convert(const T& value)
	{
		return convert_(value);
	}

private:
	static std::wstring convert_(char value)
	{
		return std::wstring(1, wchar_t(value));
	}

	static std::wstring convert_(const char* value)
	{
		if (value == 0)
			return convert_((void*)0);
		
		std::wstring result;
		result.resize(std::strlen(value));
		std::size_t size = std::mbstowcs(const_cast<wchar_t*>(result.data()), value, result.size());
		if (size == std::size_t(-1))
			result.clear();
		else
			result.resize(size);
		return result;
	}	

	static std::wstring convert_(const wchar_t* value)
	{
		if (value == 0)
			return convert((void*)0);

		return value;
	}

	template<std::size_t N>
	static std::wstring convert_(const char (&value)[N])
	{
		return convert_((const char*)value);
	}

	/// append runtime info, char type (char, string ect) should use as<char> method
	/// @param value 
	template< typename T>
	static std::wstring convert_(const T& value)
	{
        std::wstringstream sstr;
        sstr <<value;
        return sstr.str();

		//return boost::lexical_cast<std::wstring>(value);
	}

	static std::wstring convert_(const std::string& value)
	{
		return convert_(value.c_str());
	}
};
}	//namespace private_

template<typename CharT, typename T>
std::basic_string<CharT> string_cast(const T& value)
{
	return private_::string_converter<CharT>::convert(value);
}

}

#endif //end AIO_STRING_CAST_H

