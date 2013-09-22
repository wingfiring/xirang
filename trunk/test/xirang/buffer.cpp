/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang/buffer.h>

BOOST_AUTO_TEST_SUITE(buffer_suite)
using namespace xirang;

BOOST_AUTO_TEST_CASE(buffer_case)
{
	buffer<int> empty;
	BOOST_CHECK(empty.capacity() == 0);
	BOOST_CHECK(empty.size() == 0);
	BOOST_CHECK(empty.empty());
	BOOST_CHECK(empty.begin() == empty.end());

	buffer<int> value(10, 3);
	BOOST_CHECK(value.size() == 10);
	BOOST_CHECK(value[0] == 3);
	value = buffer<int>(10, 5);
	BOOST_CHECK(value[0] == 5);

	buffer<int> value2(to_range(value));
	BOOST_CHECK(value2.size() == 10);
	BOOST_CHECK(value2[0] == 5);

	value.assign(10, 3);
	BOOST_CHECK(value[0] == 3);
	value = to_range(value2);
	BOOST_CHECK(value[0] == 5);

	value.reserve(100);
	BOOST_CHECK(value.capacity() >= 100);
	value.resize(20);
	BOOST_CHECK(value.size() == 20);

	value.assign(10, 3);
	value.insert(value.begin(), to_range(value2));
	BOOST_CHECK(value[0] == 5);
	BOOST_CHECK(value.size() == 20);

	value.insert(value.begin() + 5, 10, 6);
	BOOST_CHECK(value[5] == 6);
	BOOST_CHECK(value.size() == 30);

	value.append(10, 7);
	BOOST_CHECK(value.size() == 40);
	value.append(to_range(value2));
	BOOST_CHECK(value.size() == 50);
	value.push_back(8);
	BOOST_CHECK(value.size() == 51);

	value.erase(value.begin());
	BOOST_CHECK(value.size() == 50);
	value.erase(make_range(value.begin() + 5, value.begin() + 15));
	BOOST_CHECK(value.size() == 40);

	value.replace(make_range(value.begin() + 5, value.begin() + 10), to_range(value2));
	BOOST_CHECK(value.size() == 45);

	value.sub(make_range(value.begin() + 10, value.begin() + 20));
	BOOST_CHECK(value.size() == 10);

	value.clear();
	BOOST_CHECK(value.empty());

}

BOOST_AUTO_TEST_SUITE_END()
