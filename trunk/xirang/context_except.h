//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_CONTEXT_EXCEPTION_H
#define AIO_CONTEXT_EXCEPTION_H
#include <xirang/config.h>
#include <xirang/macro_helper.h>
#include <xirang/utf8char.h>
#include <xirang/string.h>

#include <xirang/config/abi_prefix.h>
namespace xirang
{
	extern basic_string<char_utf8> line2string(unsigned line);

	struct stack_info_tag{};
	const stack_info_tag stack_info;

	//used to collect exception point runtime information
	// @param base usually, user just need to define a empty exception class
	// with or without base class. all xirang exception class should derived from ::xirang::exception
	template<typename Base>
	class context_exception : public Base
	{
	public:
		explicit context_exception(const char_utf8* file, unsigned lineno)
			: m_msg(file)
		{
			m_msg.push_back('(');
			m_msg += line2string(lineno);
			m_msg.push_back(')');
		}
		/// just for nothow compatible
		virtual ~context_exception() AIO_COMPATIBLE_NOTHROW() {}

		/// append runtime info, char type (char, string ect) 
		/// @param value 
		context_exception& operator()(const const_range_string& value)
		{
			this->m_msg += value;
			return *this;
		}

		/// append runtime info, char type (char, string ect) should use as<char> method
		/// @param value 
		context_exception& operator()(const char_utf8* value)
		{
			if (value != 0)
				this->m_msg += value;
			return *this;
		}

		context_exception& operator()(stack_info_tag){
			//TODO: imp
			return *this;
		}

		/// @return exception information, return as  char string
		virtual const char_utf8* what() const  AIO_COMPATIBLE_NOTHROW() {
			return this->m_msg.c_str();
		}

		virtual context_exception& append(const char_utf8* info) {
			return (*this)(info);
		}

		virtual context_exception& append(stack_info_tag info) {
			return (*this)(info);
		}
	private:
		basic_string_builder<char_utf8> m_msg;
	};
}

#define AIO_THROW(ex) throw ::xirang::context_exception<ex>(__FILE__, __LINE__)

#include <xirang/config/abi_suffix.h>
#endif //end AIO_CONTEXT_EXCEPTION_H
