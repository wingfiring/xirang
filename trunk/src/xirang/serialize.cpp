#include <aio/xirang/serialize.h>
#include <boost/numeric/conversion/cast.hpp>

namespace xirang{namespace lio{
	extern uint32_t numeric_uint32_cast(size_t n){
		return boost::numeric_cast<uint32_t>(n);
	}

}}


