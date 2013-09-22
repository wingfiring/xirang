
/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang/string_algo/utf8.h>
#include <xirang/string_algo/uri.h>
#include <xirang/string.h>
#include <xirang/buffer.h>

//STL
#include <vector>

#include <iostream>

BOOST_AUTO_TEST_SUITE(utf8_suite)
using namespace aio;
using  std::ostream_iterator;
using  std::cout;

BOOST_AUTO_TEST_CASE(utf8_case)
{
	std::cout << std::hex;

#if 0
	std::vector<int> source =
	{
		0x1,0x81,0x801,0x10001, 0x200001,0x4000001
	};
#else
	std::vector<int> source; 
	source.push_back(0x1);
	source.push_back(0x81);
	source.push_back(0x801);
	source.push_back(0x10001);
	source.push_back(0x200001);
	source.push_back(0x4000001);
#endif

	string  result = utf8::encode_string(source);

	BOOST_CHECK(result.size() == 21);

	buffer<long> source2 = utf8::decode_to<buffer<long> >(result);
	BOOST_CHECK(!lexicographical_compare(source, source2));

	BOOST_CHECK(!lexicographical_compare(source2, source));
}

BOOST_AUTO_TEST_SUITE_END()
