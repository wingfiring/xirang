#include "precompile.h"
#include <aio/xirang/xirang.h>
#include <aio/xirang/typebinder.h>
#include <aio/xirang/binder.h>
#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang;

BOOST_AUTO_TEST_SUITE(xirang_suites)

BOOST_AUTO_TEST_CASE(xirang_smoke_case)
{
	Xirang xi("test", aio::memory::get_global_heap(), aio::memory::get_global_ext_heap());
    BOOST_CHECK(xi.name() == "test");	

    BOOST_CHECK(xi.root().valid()); 
    BOOST_CHECK(xi.root().name().empty());

    BOOST_CHECK(xi.get_heap().equal_to(aio::memory::get_global_heap())); 

    BOOST_CHECK(xi.get_ext_heap().equal_to(aio::memory::get_global_ext_heap())); 
}


BOOST_AUTO_TEST_CASE(xirang_setup_case)
{
	Xirang xi("test", aio::memory::get_global_heap(), aio::memory::get_global_ext_heap());
	SetupXirang(xi);
    
    BOOST_CHECK(xi.root().findNamespace("sys").findType("int").valid());
	BOOST_CHECK(xi.root().findType("int").valid());
	BOOST_CHECK(xi.root().findAlias("int").valid());

	BOOST_CHECK(xi.root().findType("int") == xi.root().findAlias("int").type());
	BOOST_CHECK(xi.root().findType("int") == xi.root().findNamespace("sys").findType("int"));

	Type tint = xi.root().findType("int");

	BOOST_CHECK(tint.fullName() == ".sys.int");
}

BOOST_AUTO_TEST_SUITE_END()
