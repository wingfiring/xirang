/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/string_algo/uri.h>
#include <aio/common/string.h>
#include <aio/common/buffer.h>

//STL
#include <vector>

BOOST_AUTO_TEST_SUITE(uri_suite)
using namespace aio;

BOOST_AUTO_TEST_CASE(uri_case)
{
	string source ="abc 123\t";
	string encoded = "abc%20123%09";

	string  result = uri::encode_string(source);

	BOOST_CHECK(result == encoded);


	buffer<long> source2 = uri::decode_to<buffer<long> >(result);
	BOOST_CHECK(!lexicographical_compare(source, source2));
	BOOST_CHECK(!lexicographical_compare(source2, source));
}

BOOST_AUTO_TEST_SUITE_END()