#include "precompile.h"
#include <xirang/xirang.h>
#include <xirang/typealias.h>

#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang;

BOOST_AUTO_TEST_SUITE(xirang_type_suites)

BOOST_AUTO_TEST_CASE(typealias_case)
{
	Xirang xi("typealias_case", aio::memory::get_global_heap(), aio::memory::get_global_ext_heap());

    SetupXirang(xi);

    TypeAliasBuilder builder;
    builder.name("int_alias")
        .typeName(".sys.int")
        .setType(xi.root().findType("int"));
    TypeAlias tmp = builder.get();

    BOOST_CHECK(tmp.valid());
    BOOST_CHECK(builder.getName() == "int_alias");
    BOOST_CHECK(builder.getTypeName() == ".sys.int");

    TypeAlias int_alias = builder.adoptBy(xi.root());    

    BOOST_REQUIRE(int_alias.valid());
    BOOST_CHECK(int_alias == tmp);

}


BOOST_AUTO_TEST_SUITE_END()
