/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang/ref_ptr.h>
#include <xirang/assert.h>

//BOOST
#include <boost/mpl/list.hpp>

//STL
#include <string>


BOOST_AUTO_TEST_SUITE(ref_ptr_suite)

using namespace xirang;

typedef boost::mpl::list<int, const int, std::string, const std::string> test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(ref_ptr_case, T, test_types)
{
	typedef ref_ptr<T> ref_type;

	T t = T();
	
	ref_type h1(t);

	BOOST_CHECK(&h1.get() == &t);
	BOOST_CHECK((T&)h1 == t);

	T v = T();
	ref_type h2(v);

	swap(h1, h2);
	BOOST_CHECK(&h1.get() == &v);
}

BOOST_AUTO_TEST_SUITE_END()
