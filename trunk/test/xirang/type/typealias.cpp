#include "../precompile.h"
#include <xirang/type/xirang.h>
#include <xirang/type/typealias.h>

#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang;
using namespace xirang::type;

BOOST_AUTO_TEST_SUITE(xirang_type_suites)

BOOST_AUTO_TEST_CASE(typealias_case)
{
	Xirang xi("typealias_case", xirang::memory::get_global_heap(), xirang::memory::get_global_ext_heap());

    SetupXirang(xi);

    TypeAliasBuilder builder;
    builder.name("int_alias")
        .typeName(".sys.int")
        .setType(xi.root().findType("int"));
    TypeAlias tmp = builder.get();

    BOOST_CHECK(tmp.valid());
    BOOST_CHECK(builder.getName() == literal("int_alias"));
    BOOST_CHECK(builder.getTypeName() == literal(".sys.int"));

    TypeAlias int_alias = builder.adoptBy(xi.root());    

    BOOST_REQUIRE(int_alias.valid());
    BOOST_CHECK(int_alias == tmp);

}


BOOST_AUTO_TEST_SUITE_END()
