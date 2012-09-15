#include <aio/common/context_except.h>
#include <aio/common/string_cast.h>
#include <sstream>
#include <string>

namespace aio {

	basic_string<char_utf8> line2string(unsigned line)
	{
        std::stringstream sstr;
        sstr << line;
		return basic_string<char_utf8>(sstr.str().c_str());	
	}
}
