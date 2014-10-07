#ifndef XIRANG_UTILITY_MAKE_REVERSE_ITERATOR_H
#define XIRANG_UTILITY_MAKE_REVERSE_ITERATOR_H

#include <xirang/config.h>
#include <iterator>
namespace xirang{
	template<typename Itr> std::reverse_iterator<Itr> make_reverse_iterator(Itr itr){
		return std::reverse_iterator<Itr>(std::move(itr));
	}
}


#endif //end XIRANG_UTILITY_MAKE_REVERSE_ITERATOR_H

