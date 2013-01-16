/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/zip.h>
#include <aio/common/archive/mem_archive.h>
#include <random>


BOOST_AUTO_TEST_SUITE(archive_adaptor_suite)
using namespace aio;
using namespace aio::io;
BOOST_AUTO_TEST_CASE(deflate_case)
{
	std::mt19937 engin;
	std::uniform_int_distribution<unsigned int> distribution;
	mem_archive mar;
	iref<reader, writer, read_map> imar(mar);
	writer& wr = imar.get<writer>();

	for (int i = 0; i < 1000000; ++i){
		unsigned int var = distribution(engin);
		aio::sio::save(wr, var);
	}

	mem_archive zipped;
	iref<writer> zipar(zipped);
	mar.seek(0);
	auto res = zip::deflate(imar.get<reader>(), zipar.get<writer>());
	BOOST_CHECK(res.err == zip::ze_ok);
	BOOST_CHECK(res.in_size == mar.size());
	BOOST_CHECK(res.out_size == zipped.size());


	mem_archive zipped2;
	iref<write_map> zipar2(zipped2);
	auto res2 = zip::deflate(imar.get<read_map>(), zipar2.get<write_map>());
	BOOST_CHECK(res2.err == zip::ze_ok);
	BOOST_CHECK(res2.in_size == mar.size());
	BOOST_CHECK(res2.out_size == res.out_size);
	zipped2.truncate(res2.out_size);

	BOOST_CHECK(zipped.data() == zipped2.data());


}
BOOST_AUTO_TEST_SUITE_END()
