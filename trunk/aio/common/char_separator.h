//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_CHAR_SEPARATOR_H
#define AIO_CHAR_SEPARATOR_H

#include <aio/common/string.h>

namespace aio
{
    enum empty_token_policy { drop_empty_tokens, keep_empty_tokens };

	template<typename CharT>
    class char_separator
    {
        typedef const_range_string string_type;
        typedef CharT char_type;
    public:
        explicit  char_separator(char_type dropped_delims,
            char_type kept_delims = 0,
            empty_token_policy empty_tokens = drop_empty_tokens)
            : m_kept_delims(kept_delims),
            m_dropped_delims(dropped_delims),
            m_empty_tokens(empty_tokens),
            m_output_done(false)
        {   
        }

        // use ispunct() for kept delimiters and isspace for dropped.
        char_separator()
            : m_empty_tokens(drop_empty_tokens) { }

        void reset() { }

        
        bool operator()(string::const_iterator & next, string::const_iterator& end, string_type& tok)
        {
            tok = string_type();

            // skip past all dropped_delims
            if (m_empty_tokens == drop_empty_tokens)
                for (; next != end  && is_dropped(*next); ++next){ };

            string::const_iterator start(next);

            if (m_empty_tokens == drop_empty_tokens) {

                if (next == end)
                    return false;

                // if we are on a kept_delims move past it and stop
                if (is_kept(*next)) {
                    tok = string_type(start, ++next);
                } 
                else {
                    // append all the non delim characters
                    for (; next != end && !is_dropped(*next) && !is_kept(*next); ++next);

                    tok = string_type(start, next);
                }
            } 
            else { // m_empty_tokens == keep_empty_tokens

                // Handle empty token at the end
                if (next == end)
                {
                    if (!m_output_done) 
                    {
                        m_output_done = true;
                        tok = string_type(start, next);
                        return true;
                    } 
                    else
                        return false;
                }

                if (is_kept(*next)) {
                    if (!m_output_done)
                        m_output_done = true;
                    else {
                        tok = string_type(start, ++next);
                        m_output_done = false;
                    }
                } 
                else if (!m_output_done && is_dropped(*next)) {
                    m_output_done = true;
                } 
                else {
                    if (is_dropped(*next))
                        start=++next;
                    for (; next != end && !is_dropped(*next) && !is_kept(*next); ++next)
                        ;
                    tok = string_type(start, next);
                    m_output_done = true;
                }
            }
            tok = string_type(start, next);
            return true;
        }

    private:
        char_type m_kept_delims;
        char_type m_dropped_delims;
        empty_token_policy m_empty_tokens;
        bool m_output_done;

        bool is_kept(char_type E) const
        {  
            return m_kept_delims == E;
        }
        bool is_dropped(char_type E) const
        {
            return m_dropped_delims == E;
        }
    };
}
#endif //end AIO_CHAR_SEPARATOR_H
