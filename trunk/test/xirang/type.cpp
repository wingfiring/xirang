#include "precompile.h"
#include <aio/xirang/xirang.h>
#include <aio/xirang/type.h>

#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang;

BOOST_AUTO_TEST_SUITE(xirang_type_suites)

BOOST_AUTO_TEST_CASE(type_case)
{
	Xirang xi("type_case", aio::memory::get_global_heap(), aio::memory::get_global_ext_heap());

    SetupXirang(xi);

    NamespaceBuilder().name("test").adoptBy(xi.root());
    Namespace test = xi.root().findNamespace("test");

    Type pair = TypeBuilder().name("pair")
        .setArg("first_type", "", Type())
        .setArg("second_type", "", Type())
        .addMember("first", "first_type", Type())
        .addMember("second", "second_type", Type())
        .endBuild()
        .adoptBy(test);

    BOOST_REQUIRE(pair.valid());
    BOOST_REQUIRE(pair);
    BOOST_CHECK(pair.name() == "pair");
    BOOST_CHECK(pair.fullName() == ".test.pair");
    BOOST_CHECK(!pair.isMemberResolved());
    BOOST_CHECK(pair.unresolvedArgs() == 2);
    BOOST_CHECK(!pair.isComplete());
    BOOST_CHECK(!pair.hasModel());
    BOOST_CHECK(!pair.model().valid());

    BOOST_CHECK(pair.memberCount() == 2);
    
    TypeItemRange::iterator member_pos = pair.members().begin();
    BOOST_REQUIRE(member_pos != pair.members().end());

    BOOST_CHECK(*member_pos == pair.member(0));
    TypeItem first_member = *member_pos;
    BOOST_REQUIRE(first_member.valid());
    BOOST_CHECK(!first_member.type().valid());
    BOOST_CHECK(first_member.name() == "first");
    BOOST_CHECK(first_member.typeName() == "first_type");
    BOOST_CHECK(first_member.index() == 0);
    BOOST_CHECK(first_member.offset() == Type::unknown);
    BOOST_CHECK(!first_member.isResolved());
    BOOST_CHECK(first_member == pair.member(0));
    BOOST_CHECK(first_member != pair.member(1));

    ++member_pos;
    BOOST_CHECK(member_pos != pair.members().end());
    BOOST_CHECK(++member_pos == pair.members().end());

    BOOST_CHECK(pair.argCount() == 2);

    TypeArgRange::iterator arg_pos = pair.args().begin();
    BOOST_REQUIRE(arg_pos != pair.args().end());

    BOOST_CHECK(*arg_pos == pair.arg(0));
    TypeArg first_arg = *arg_pos;
    BOOST_REQUIRE(first_arg.valid());
    BOOST_CHECK(!first_arg.type().valid());
    BOOST_CHECK(first_arg.name() == "first_type");
    BOOST_CHECK(first_arg.typeName().empty());
    BOOST_CHECK(!first_arg.isBound());
    BOOST_CHECK(first_arg == pair.arg(0));
    BOOST_CHECK(first_arg != pair.arg(1));

    Type type_int = xi.root().locateType("int", '.');
    Type pair_int = TypeBuilder().name("pair_int")
        .modelFrom(pair)
        .setArg("first_type", "int", type_int)
        .endBuild()
        .adoptBy(test);

    TypeArg first_int_arg = pair_int.arg(0);
    BOOST_REQUIRE(first_int_arg.valid());
    BOOST_CHECK(first_int_arg.type().valid());
    BOOST_CHECK(first_int_arg.type() == type_int);
    BOOST_CHECK(first_int_arg.name() == "first_type");
    BOOST_CHECK(first_int_arg.typeName() == "int");
    BOOST_CHECK(first_int_arg.isBound());

    TypeItem first_int_member = pair_int.member(0);
    BOOST_REQUIRE(first_int_member.valid());
    BOOST_CHECK(first_int_member.type() == type_int);
    BOOST_CHECK(first_int_member.typeName() == "first_type");
    BOOST_CHECK(first_int_member.offset() == 0);
    BOOST_CHECK(first_int_member.isResolved());

    BOOST_CHECK(!pair_int.isComplete());
    BOOST_CHECK(!pair_int.isMemberResolved());
    BOOST_CHECK(pair_int.unresolvedArgs() == 1);
    BOOST_CHECK(pair_int.hasModel());
    BOOST_CHECK(pair_int.model() == pair);
    BOOST_CHECK(pair_int.modelName() == "pair");

    Type type_long = xi.root().locateType("long", '.');
    Type pair_int_long = TypeBuilder().name("pair_int_long")
        .modelFrom(pair_int)
        .setArg("second_type", "long", type_long)
        .endBuild()
        .adoptBy(test);
    
    struct IntLongPair_{
        int m1;
        long m2;
    };

    BOOST_CHECK(pair_int_long.isComplete());
    BOOST_CHECK(pair_int_long.isMemberResolved());
    BOOST_CHECK(pair_int_long.unresolvedArgs() == 0);
    BOOST_CHECK(pair_int_long.hasModel());
    BOOST_CHECK(pair_int_long.model() == pair);
    BOOST_CHECK(pair_int_long.modelName() == "pair");
/*
    BOOST_CHECK(pair_int_long.align() == STDTR1::alignment_of<IntLongPair_>::value);
 */
    BOOST_CHECK(pair_int_long.payload() == sizeof(IntLongPair_));
    BOOST_CHECK(pair_int_long.isPod());

    first_int_member = pair_int_long.member(0);
    BOOST_REQUIRE(first_int_member.valid());
    BOOST_CHECK(first_int_member.type() == type_int);
    BOOST_CHECK(first_int_member.offset() == 0);
    BOOST_CHECK(first_int_member.isResolved());

    TypeItem second_long_member = pair_int_long.member(1);
    BOOST_REQUIRE(second_long_member.valid());
    BOOST_CHECK(second_long_member.type() == type_long);
    BOOST_CHECK(second_long_member.isResolved());
    BOOST_CHECK(second_long_member.offset() == sizeof(int));
    BOOST_CHECK(second_long_member.index() == 1);

    //
    Type pair_int_string = TypeBuilder().name("pair_int_string")
        .modelFrom(pair_int)
        .setArg("second_type", "string", xi.root().locateType("string",'.'))
        .endBuild()
        .adoptBy(test);
    BOOST_CHECK(pair_int_string.valid() && pair_int_string.isMemberResolved() && !pair_int_string.isPod());

    //test LocateType for nested type
    Type triple_int_pair_long = TypeBuilder().name("triple_int_pair_long")
        .modelFrom(pair_int)
        .setArg("second_type", "pair_int_long", pair_int_long)
        .addMember("thrid", "second_type.second_type", Type())
        .endBuild()
        .adoptBy(test);

    BOOST_REQUIRE(triple_int_pair_long.valid() && triple_int_pair_long.isMemberResolved());
    BOOST_CHECK(triple_int_pair_long.member(1).type() == pair_int_long);
    BOOST_CHECK(triple_int_pair_long.member(2).type() == type_long);
}


BOOST_AUTO_TEST_SUITE_END()
