/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/archive/mem_archive.h>
#include <aio/common/archive/adaptor.h>

#include "./iarchive.h"

BOOST_AUTO_TEST_SUITE(archive_adaptor_suite)
using namespace aio;
using namespace aio::io;

template<template<typename> class ArchiveData, typename... Interface>
struct test_adaptor{
	template<template<typename> class ... Partial>
	static	void apply(){
			mem_archive mar;
			mar.data().resize(1024);
			iref <Interface...> someface(mar);
			typedef ArchiveData<iref<Interface...>> archive_data;
			auto adaptor = decorate2<archive_data, Partial...>(someface);
		}
};

BOOST_AUTO_TEST_CASE(dummy_case)
{
	test_adaptor<proxy_archive, reader>::apply<proxy_reader_p>(); 
	test_adaptor<proxy_archive, writer>::apply<proxy_writer_p>(); 
	test_adaptor<proxy_archive, ioctrl>::apply<proxy_ioctrl_p>(); 
	test_adaptor<proxy_archive, ioinfo>::apply<proxy_ioinfo_p>(); 
	test_adaptor<proxy_archive, sequence>::apply<proxy_sequence_p>(); 
	test_adaptor<proxy_archive, forward>::apply<proxy_forward_p>(); 
	test_adaptor<proxy_archive, io::random>::apply<proxy_random_p>(); 
	test_adaptor<proxy_archive, read_map>::apply<proxy_read_map_p>(); 
	test_adaptor<proxy_archive, write_map>::apply<proxy_write_map_p>(); 

	test_adaptor<proxy_archive, reader, io::random>::apply<proxy_reader_p, proxy_forward_p>(); 
}
BOOST_AUTO_TEST_CASE(multiplex_archive_case)
{
	mem_archive mar;
	mar.data().resize(1024);
	typedef iref <reader,writer, io::random, read_map, write_map> interface_type;
	interface_type iar(mar);

	auto adaptor = decorate<multiplex_archive
		, multiplex_reader_p
		, multiplex_writer_p
		, multiplex_random_p
		, multiplex_read_map_p
		, multiplex_write_map_p
		>(iar, 0);

	auto adaptor2 = decorate<multiplex_archive
		, multiplex_reader_p
		, multiplex_writer_p
		, multiplex_random_p
		, multiplex_read_map_p
		, multiplex_write_map_p
		>(mar, 0);

	auto adaptor3 = decorate<multiplex_archive
		, multiplex_reader_p
		, multiplex_writer_p
		, multiplex_random_p
		, multiplex_read_map_p
		, multiplex_write_map_p
		>(mem_archive(), 0);

	int var = 42;
	sio::save(adaptor, var);
	int var2 = sio::load<int>(adaptor2);
	BOOST_CHECK(var == var2);
}

BOOST_AUTO_TEST_CASE(sub_archive_case)
{
	mem_archive mar;
	mar.data().resize(1024);
	{
		typedef iref <reader,writer, io::random, read_map, write_map > interface_type;
		interface_type iar(mar);
		auto adaptor = decorate<sub_archive
			, sub_reader_p
			, sub_writer_p
			, sub_random_p
			, sub_read_map_p
			, sub_write_map_p
			>(iar, 0, 10);

	}
	typedef iref <reader,writer, io::random > interface_type;
	interface_type iar(mar);

	auto mul_adaptor1 = decorate<multiplex_archive
		, multiplex_reader_p
		, multiplex_writer_p
		, multiplex_random_p
		>(iar, 0);

	auto mul_adaptor2 = mul_adaptor1;
	interface_type iar1(mul_adaptor1);
	interface_type iar2(mul_adaptor2);

	auto adaptor = decorate<sub_archive
		, sub_reader_p
		, sub_writer_p
		, sub_random_p
		>(iar1, 0, 10);

	iar2.get<io::random>().seek(4);
	auto adaptor2 = decorate<sub_archive
		, sub_reader_p
		, sub_writer_p
		, sub_random_p
		>(iar2, 4, 10);

	int var = 42;
	sio::save(adaptor, var);
	sio::save(adaptor, var);
	int var2 = sio::load<int>(adaptor2);
	BOOST_CHECK(var == var2);
}

BOOST_AUTO_TEST_CASE(tail_archive_case)
{
	mem_archive mar;
	mar.data().resize(1024);
	typedef iref <reader,writer, io::random, read_map, write_map> interface_type;
	interface_type iar(mar);
	mar.seek(1000);

	auto adaptor = decorate<tail_archive
		, tail_reader_p
		, tail_writer_p
		, tail_random_p
		, tail_read_map_p
		, tail_write_map_p
		>(iar, 1000);

	BOOST_CHECK(static_cast<io::random&>(adaptor).size() == 24);
}

BOOST_AUTO_TEST_SUITE_END()

