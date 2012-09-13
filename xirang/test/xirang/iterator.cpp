#include "precompile.h"
#include <aio/common/iterator.h>
#include <aio/xirang/xrfwd.h>
#include <vector>
using namespace xirang;
BOOST_AUTO_TEST_SUITE(ut_xirang)

BOOST_AUTO_TEST_CASE( iterator_case)
{
	std::vector<int> vec;
	for (int i = 0; i < 10; ++i)
		vec.push_back(i);

	RangeT<aio::default_itr_traits<int> > r (aio::IteratorT<aio::default_itr_traits<int> >(vec.begin())
        , aio::IteratorT<aio::default_itr_traits<int> >(vec.end()));

	RangeT<aio::default_itr_traits<int> >::iterator p;
	BOOST_CHECK(!p.valid());
   	p = r.begin();
	BOOST_REQUIRE(p.valid());

	BOOST_CHECK(*(p++) == 0);
	BOOST_CHECK(*++p == 2);
	BOOST_CHECK(*--p == 1);
	BOOST_CHECK(*(p--) == 1);
	
	RangeT<aio::default_itr_traits<int> >::iterator p2 = p++;
	BOOST_CHECK(*p2 == 0 && *p == 1);
	p2.swap(p);
	BOOST_CHECK(*p2 == 1 && *p == 0);

	BOOST_CHECK(!p2.equals(p));
	--p2;
	BOOST_CHECK(p2.equals(p));
}

BOOST_AUTO_TEST_SUITE_END()
