#include <aio/common/context_except.h>
#include <aio/common/to_string.h>
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