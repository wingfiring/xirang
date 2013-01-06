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
			auto adapter = combine<archive_data, Partial...>(someface);
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
	test_adaptor<proxy_archive, reader, io::random>::apply<proxy_reader_p, proxy_forward_p>(); 


}
BOOST_AUTO_TEST_SUITE_END()
