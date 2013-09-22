#ifndef AIO_COMMON_STRING_ALGO_UTF8_H
#define AIO_COMMON_STRING_ALGO_UTF8_H

#include <xirang/config.h>
#include <xirang/utf8char.h>
#include <xirang/context_except.h>

//STL
#include <iterator>
#include <type_traits> //for: make_unsigned

#ifdef MSVC_COMPILER_
#pragma warning(disable: 4333 4244)
#endif
namespace aio{ namespace utf8 {

	AIO_EXCEPTION_TYPE(unicode_out_range);
	AIO_EXCEPTION_TYPE(invalid_utf8_code);
	AIO_EXCEPTION_TYPE(no_enough_utf8_input);

	namespace private_{

		inline void check_throw(bool need_throw)
		{
			if (need_throw)
				AIO_THROW(no_enough_utf8_input);
		}

		template<typename InputIter> unsigned int get_char(const InputIter& itr)
		{
			return (*itr & 0xff);
		}

		template<typename Itr>
		struct value_type {
			typedef typename Itr::value_type type;
		};

		template<typename Cont>
		struct value_type< std::back_insert_iterator<Cont> >
		{
			typedef typename Cont::value_type type;
		};
	}

	template<typename T>
	typename std::make_unsigned<T>::type to_unsigned(T t){
		typedef typename std::make_unsigned<T>::type return_type;
		return static_cast<return_type>(t);
	}

	template<typename InputIter, typename OutputIter>
		std::pair<InputIter, OutputIter> encode(range<InputIter> in, OutputIter out)
		{
			InputIter iin = in.begin();
            
			typedef unsigned long value_type;
			for (; iin != in.end(); ++iin, ++out)
			{
				value_type ch = to_unsigned(*iin);
				if (ch < 0x80)
				{
					//U+00000000 – U+0000007F	0xxxxxxx
					*out = ch;
				}
				else if(ch < 0x800)
				{
					//U+00000080 – U+000007FF	110xxxxx 10xxxxxx
					*out = 0xc0 | (ch >> 6);
					*++out = 0x80 | ( ch & 0x3f);
				}
				else if(ch < 0x10000)
				{
					//U+00000800 – U+0000FFFF	1110xxxx 10xxxxxx 10xxxxxx
					*out = 0xe0 | (ch >> 12);
					*++out = 0x80 | ( (ch >> 6)& 0x3f);
					*++out = 0x80 | ( ch & 0x3f);
				}
				else if(ch < 0x200000)
				{
					//U+00010000 – U+001FFFFF	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
					*out = 0xf0 | (ch >> 18);
					*++out = 0x80 | ( (ch >> 12)& 0x3f);
					*++out = 0x80 | ( (ch >> 6)& 0x3f);
					*++out = 0x80 | ( ch & 0x3f);
				}
				else if (ch < 0x4000000)
				{
					//U+00200000 – U+03FFFFFF	111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
					*out = 0xf8 | (ch >> 24);
					*++out = 0x80 | ( (ch >> 18)& 0x3f);
					*++out = 0x80 | ( (ch >> 12)& 0x3f);
					*++out = 0x80 | ( (ch >> 6)& 0x3f);
					*++out = 0x80 | ( ch & 0x3f);
				}
				else if (ch < value_type(0x80000000U))
				{
					//U+04000000 – U+7FFFFFFF	1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
					*out = 0xfc | (ch >> 30);
					*++out = 0x80 | ( (ch >> 24)& 0x3f);
					*++out = 0x80 | ( (ch >> 18)& 0x3f);
					*++out = 0x80 | ( (ch >> 12)& 0x3f);
					*++out = 0x80 | ( (ch >> 6)& 0x3f);
					*++out = 0x80 | ( ch & 0x3f);
				}
				else
				{
					AIO_THROW(unicode_out_range)("aio::utf8::encode");
				}
			}
			return std::make_pair(iin, out);
		};

	template<typename InputIter, typename OutputIter>
		std::pair<InputIter, OutputIter> decode(range<InputIter> rin, OutputIter out)
		{
			typedef typename private_::value_type<OutputIter>::type  out_value_type;
			InputIter in = rin.begin();
			InputIter end = rin.end();
			for (; in != end; ++in, ++out)
			{
				using private_::get_char;
				using private_::check_throw;
				unsigned long ch = get_char(in);
				if (ch < 0x80)
				{
					*out = ch;
				}
				else if(ch < 0xe0)
				{
					out_value_type t = ch & ~0xe0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					*out = t;
				}
				else if(ch < 0xf0)
				{
					out_value_type t = ch & ~0xf0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					*out = t;
				}
				else if (ch < 0xf8)
				{
					out_value_type t = ch & ~0xf8;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					*out = t;
				}
				else if (ch < 0xfc)
				{
					out_value_type t = ch & ~0xfc;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					*out = t;
				}
				else if (ch < 0xfe)
				{
					out_value_type t = ch & ~0xfe;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					t <<= 6;
					check_throw (++in == end);
					t |= get_char(in) & ~0xc0;
					*out = t;
				}
				else
				{
					AIO_THROW(invalid_utf8_code);
				}

			}
			return std::make_pair(in, out);
		}

	template<typename Cont, typename SrcCont>
		Cont encode_to(const SrcCont& rhs)
		{
			Cont sb;
			encode(to_range(rhs), std::back_inserter(sb));
			return sb;
		}

	template<typename Cont, typename SrcCont>
		Cont decode_to(const SrcCont& rhs)
		{
			Cont sb;
			decode(to_range(rhs), std::back_inserter(sb));
			return sb;
		}

	template<typename SrcCont>
		string encode_string(const SrcCont& rhs)
		{
			string_builder sb;
			encode(to_range(rhs), std::back_inserter(sb));
			return string(sb);
		}

	template<typename SrcCont>
		wstring decode_string(const SrcCont& rhs)
		{
			wstring_builder sb;
			decode(to_range(rhs), std::back_inserter(sb));
			return wstring(sb);
		}

}

}
#endif //end AIO_COMMON_STRING_ALGO_UTF8_H
