//XIRANG_LICENSE_PLACE_HOLDER
#include "precompile.h"
#include <xirang/mpl.h>

#include <iostream>
#include <array>
#include <typeinfo>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(mpl_suite)

struct A{ static const int value = 1; };
struct B{ static const int value = 2; };
struct C{ static const int value = 3; };

template<typename T, typename U>
struct Less{
	static const bool value = T::value < U::value;
};

BOOST_AUTO_TEST_CASE(mpl_case)
{
	using namespace xirang::mpl;
	using std::cout;
	using std::endl;
	
	typedef seq<> empty_seq;
	BOOST_CHECK(0 == size<empty_seq>::value);
	
	typedef seq<B,A,C> seq;
	BOOST_CHECK(size<seq>::value == 3);
	BOOST_CHECK((at<1, seq>::type::value == 1));
	BOOST_CHECK(first<seq>::type::value == 2);
	BOOST_CHECK(last<seq>::type::value == 3);
	BOOST_CHECK((size<concat<seq, seq>::type>::value == 6));

	typedef sort<seq, Less>::type less_sorted;
	BOOST_CHECK((at<0, less_sorted>::type::value == 1));
	BOOST_CHECK((at<1, less_sorted>::type::value == 2));
	BOOST_CHECK((at<2, less_sorted>::type::value == 3));
}

BOOST_AUTO_TEST_SUITE_END()
