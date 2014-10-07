#ifndef XIRANG_UTILITY_INDIRECT_LESS
#define XIRANG_UTILITY_INDIRECT_LESS
#include <xirang/config.h>
namespace xirang{
	struct indirect_less
	{
		template<typename T>
		bool operator()(const T& lhs, const T& rhs)const
		{
			return *lhs < *rhs;
		}
	};
}

#endif //end XIRANG_UTILITY_INDIRECT_LESS



