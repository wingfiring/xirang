#include "../precompile.h"
#include <xirang/type/xirang.h>
#include <xirang/type/typebinder.h>
#include <xirang/type/binder.h>
#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang;
using namespace xirang::type;

BOOST_AUTO_TEST_SUITE(xirang_suites)

BOOST_AUTO_TEST_CASE(xirang_smoke_case)
{
	Xirang xi("test", xirang::memory::get_global_heap(), xirang::memory::get_global_ext_heap());
    BOOST_CHECK(xi.name() == literal("test"));	

    BOOST_CHECK(xi.root().valid()); 
    BOOST_CHECK(xi.root().name().empty());

    BOOST_CHECK(xi.get_heap().equal_to(xirang::memory::get_global_heap())); 

    BOOST_CHECK(xi.get_ext_heap().equal_to(xirang::memory::get_global_ext_heap())); 
}


BOOST_AUTO_TEST_CASE(xirang_setup_case)
{
	Xirang xi("test", xirang::memory::get_global_heap(), xirang::memory::get_global_ext_heap());
	SetupXirang(xi);
    
    BOOST_CHECK(xi.root().findNamespace("sys").findType("int").valid());
	BOOST_CHECK(xi.root().findType("int").valid());
	BOOST_CHECK(xi.root().findAlias("int").valid());

	BOOST_CHECK(xi.root().findType("int") == xi.root().findAlias("int").type());
	BOOST_CHECK(xi.root().findType("int") == xi.root().findNamespace("sys").findType("int"));

	Type tint = xi.root().findType("int");

	BOOST_CHECK(tint.fullName() == literal(".sys.int"));
}

BOOST_AUTO_TEST_SUITE_END()
