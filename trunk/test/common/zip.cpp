/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/deflate.h>
#include <aio/common/io/memory.h>
#include <aio/common/io/s11n.h>
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

	for (int i = 0; i < 100; ++i){
		unsigned int var = distribution(engin);
		save(local::as_sink(wr), var);
	}

	mem_archive zipped;
	iref<writer, reader> zipar(zipped);
	mar.seek(0);
	auto res = zip::deflate(imar.get<reader>(), zipar.get<writer>());
	BOOST_CHECK(res.err == zip::ze_ok);
	BOOST_CHECK(res.in_size == mar.size());
	BOOST_CHECK(res.out_size == zipped.size());


	mem_archive zipped2;
	iref<write_map, read_map> zipar2(zipped2);
	auto res2 = zip::deflate(imar.get<read_map>(), zipar2.get<write_map>());
	BOOST_CHECK(res2.err == zip::ze_ok);
	BOOST_CHECK(res2.in_size == mar.size());
	BOOST_CHECK(res2.out_size == res.out_size);
	zipped2.truncate(res2.out_size);

	BOOST_CHECK(zipped.data() == zipped2.data());


	zipped.seek(0);
	mem_archive outar1;
	iref<writer> iout1(outar1);
	auto res3 = zip::inflate(zipar.get<reader>(), iout1.get<writer>());
	BOOST_CHECK(res3.err == zip::ze_ok);
	BOOST_CHECK(res3.in_size == zipped.size());
	BOOST_CHECK(res3.out_size == mar.size());
	BOOST_CHECK(res3.out_size == outar1.size());
	BOOST_CHECK(mar.data() == outar1.data());


	zipped2.seek(0);
	mem_archive outar2;
	iref<write_map> iout2(outar2);
	auto res4 = zip::inflate(zipar2.get<read_map>(), iout2.get<write_map>());
	BOOST_CHECK(res4.err == zip::ze_ok);
	BOOST_CHECK(res4.in_size == zipped2.size());
	BOOST_CHECK(res4.out_size == mar.size());
	outar2.truncate(res4.out_size);
	BOOST_CHECK(res4.out_size == outar2.size());
	BOOST_CHECK(mar.data() == outar2.data());

}
BOOST_AUTO_TEST_SUITE_END()
