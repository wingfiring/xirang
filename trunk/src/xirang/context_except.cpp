#include <xirang/context_except.h>
#include <xirang/to_string.h>
#include <sstream>
#include <string>

namespace aio {

	basic_string<char_utf8> line2string(unsigned line)
	{
        std::stringstream sstr;
        sstr << line;
		return basic_range_string<const char_utf8>(sstr.str());	
	}
}
