/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/handle_ptr.h>
#include <aio/common/assert.h>

//BOOST
#include <boost/mpl/list.hpp>

//STL
#include <string>


BOOST_AUTO_TEST_SUITE(handle_ptr_suite)

using namespace aio;

typedef boost::mpl::list<int, const int, std::string, const std::string> test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(handle_ptr_case, T, test_types)
{
	typedef handle_ptr<T> handle_type;

	T t = T();
	
	handle_type h1(&t);

	BOOST_REQUIRE(!h1.is_null());
	BOOST_CHECK(h1.get() == &t);
	BOOST_CHECK(&*h1 == &t);
	BOOST_CHECK(h1.operator->() == &t);

	T v = T();
	handle_type h2(&v);

	swap(h1, h2);
	BOOST_CHECK(h1.get() == &v);
	
	h1.release();
	BOOST_CHECK(h1.get() == 0);
	BOOST_CHECK(h1.is_null());

	h1.reset(&t);
	BOOST_CHECK(h1.get() == &t);

	handle_type h3;
	BOOST_CHECK(h3.is_null());
	BOOST_CHECK(h3.get() == 0);	
}

BOOST_AUTO_TEST_SUITE_END()
