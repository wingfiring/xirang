#ifndef AIO_COMMON_STRING_ALGO_STRING_H
#define AIO_COMMON_STRING_ALGO_STRING_H

#include <xirang/string.h>
#include <algorithm>

namespace xirang{ namespace str_algo {

    namespace private_{
        template<typename T>
        struct detect_iterator
        {
            typedef typename T::iterator type;
        };

        template<typename T>
        struct detect_iterator<const T>
        {
            typedef typename T::const_iterator type;
        };
    }

    template<typename CharT>
    basic_string<CharT> replace(const basic_string<CharT>& src, CharT oldv, CharT newv){
        basic_string_builder<CharT> sb(src);
        std::replace(sb.begin(), sb.end(), oldv, newv);
        return basic_string<CharT>(sb);
    }

    template<typename CharT>
    basic_string<typename std::remove_const<CharT>::type> replace(
			basic_range_string<CharT> src, basic_range_string<CharT> pattern, basic_range_string<CharT> var){
		typedef typename std::remove_const<CharT>::type CharRT;
        basic_string_builder<CharRT> sb;
        auto first = src.begin();
        auto last =src.end();
        for (; first != last;){
            auto pos = std::search( first, last, pattern.begin(), pattern.end());
            sb.append(make_range(first, pos));

            if (pos != last){
                pos += pattern.size();
                sb.append(var);
            }
            first = pos;
        }
        
        return basic_string<CharRT>(sb);
    }
    
    template<typename CharT>
    CharT toupper(CharT c){
        int ch(c);
        return ch >= 'a' && ch <= 'z' ?  CharT(ch - 'a' + 'A') : c;
    }

    template<typename CharT>
    CharT tolower(CharT c){
        int ch(c);
        return ch >= 'A' && ch <= 'Z' ?  CharT(ch - 'A' + 'a') : c;
    }
        
    template<typename CharT>
    basic_string<CharT> toupper_copy(const basic_string<CharT>& src){
        basic_string_builder<CharT> sb(src);
        std::transform(sb.begin(), sb.end(), sb.begin(), toupper<CharT> );
        return basic_string<CharT>(sb);
    }

    template<typename CharT>
    basic_string<CharT> tolower_copy(const basic_string<CharT>& src){
        basic_string_builder<CharT> sb(src);
        std::transform(sb.begin(), sb.end(), sb.begin(), tolower<CharT> );
        return basic_string<CharT>(sb);
    }

    template<typename IterT, typename ValueT>
    IterT rfind(IterT first, IterT last, ValueT var)
    {
        if (last == first)
            return last;
        IterT itr = last;
        -- itr;

        for (; itr != first; --itr)
        {
            if (*itr == var)
                return itr;
        }
        return *itr == var ? itr : last;
    }

    template<typename ContT, typename ValueT>
    typename private_::detect_iterator<ContT>::type rfind(ContT& src, ValueT var){
        return rfind(src.begin(), src.end(), var);
    }
    template<typename ContT, typename ValueT>
    typename private_::detect_iterator<const ContT>::type rfind(const ContT& src, ValueT var){
        return rfind(src.begin(), src.end(), var);
    }

    template<typename ContT, typename ValueT>
    typename private_::detect_iterator<ContT>::type find(ContT& src, ValueT var){
        return std::find(src.begin(), src.end(), var);
    }
    template<typename ContT, typename ValueT>
    typename private_::detect_iterator<const ContT>::type find(const ContT& src, ValueT var){
        return std::find(src.begin(), src.end(), var);
    }

    template<typename ContT, typename ValueT>
    bool contains(const ContT& src, ValueT var){
        return find(src, var) != src.end();
    }

}

using str_algo::replace;
using str_algo::rfind;
using str_algo::find;
using str_algo::toupper;
using str_algo::tolower;
using str_algo::toupper_copy;
using str_algo::tolower_copy;

using std::find;
using str_algo::rfind;
using str_algo::contains;
}
#endif //end AIO_COMMON_STRING_ALGO_STRING_H
