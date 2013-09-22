#include "precompile.h"
#include <xirang/xirang.h>
#include <xirang/namespace.h>
#include <xirang/typebinder.h>
#include <xirang/binder.h>
#include <xirang/object.h>
#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang;

BOOST_AUTO_TEST_SUITE(namespace_suite)

BOOST_AUTO_TEST_CASE(namespace_case)
{
	Xirang xi("test_namespace", aio::memory::get_global_heap(), aio::memory::get_global_ext_heap());
    BOOST_CHECK(xi.root().name().empty());
    BOOST_CHECK(xi.root().fullName('/') == string("/"));

	SetupXirang(xi);
    
    Namespace user = NamespaceBuilder()
        .name("user")
        .adoptBy(xi.root());
    BOOST_REQUIRE(user.valid());

    Namespace user_include = NamespaceBuilder()
        .name("include")
        .adoptBy(user);

    Namespace sys = user.locateNamespace(".sys", '.');
    BOOST_REQUIRE(sys.valid() && "locate with absolute path");
    Namespace sys2 = user.locateNamespace("/sys", '/');
    BOOST_CHECK(sys == sys2 && "locate with absolute path and use / as dim");
    Namespace sys3 = user.locateNamespace("sys", '.');
    BOOST_CHECK(sys == sys3 && "locate with relative path which in parent namespace");

    Namespace user_include2 = user.locateNamespace("include", '.');
    BOOST_REQUIRE(user_include2 == user_include && "locate in current namespace");
    BOOST_CHECK(user_include.fullName() == ".user.include");
    BOOST_CHECK(user_include.empty());
    BOOST_CHECK(user_include.parent() == user);
    BOOST_CHECK(user_include != user);
    BOOST_CHECK(user_include.root() == xi.root());


    Type type_int = sys.locateType("int", '.');
    BOOST_CHECK(type_int.valid() && "locate type via relative type name");
    BOOST_CHECK(type_int == sys.locateType("sys.int", '.') && "locate via relative type path.");
    BOOST_CHECK(type_int == user_include.locateType("sys.int", '.') && "locate via relative type path.");
    BOOST_CHECK(type_int == user_include.locateType(".sys.int", '.') && "locate via absolute type path.");

    BOOST_CHECK(!xi.root().findRealType("int").valid());
    BOOST_CHECK(sys.findRealType("int").valid());

    BOOST_CHECK(xi.root().findType("int").valid() && "find an alias in root");

    TypeAlias int_alias = xi.root().findAlias("int");
    BOOST_REQUIRE(int_alias.valid());
    BOOST_CHECK(int_alias.type() == type_int);


    BOOST_CHECK(user_include.types().begin() == user_include.types().end());

    TypeBuilder()
        .name("test")
        .addMember("m1", "int", type_int)
        .endBuild()
        .adoptBy(user);

    size_t type_counter = 0;
    TypeRange types = user.types();
    for (TypeRange::iterator itr = types.begin(); itr !=types.end(); ++itr)
        ++type_counter;

    BOOST_CHECK(type_counter == 1);

    TypeAliasBuilder().name("test_alias")
        .setType(type_int)
        .typeName("int")
        .adoptBy(user);
    size_t alias_counter = 0;
   TypeAliasRange alias = user.alias();
    for (TypeAliasRange::iterator itr = alias.begin(); itr != alias.end(); ++itr)
        ++alias_counter;
    BOOST_CHECK(alias_counter == 1);

    size_t children_counter = 0;
    NamespaceRange children = user.namespaces();
    for (NamespaceRange::iterator itr = children.begin(); itr != children.end(); ++itr)
        ++children_counter;
    BOOST_CHECK(children_counter == 1);


    ObjectFactory(xi).create(type_int, user, "int_obj");
    size_t obj_counter = 0;
    ObjectRange objects = user.objects();
    for (ObjectRange::iterator itr = objects.begin(); itr != objects.end(); ++itr)
        ++obj_counter;
    BOOST_CHECK(obj_counter == 1);
}

BOOST_AUTO_TEST_CASE(namespace_builder_case)
{
    Xirang xi("test_namespace_builder", aio::memory::get_global_heap(), aio::memory::get_global_ext_heap());
	SetupXirang(xi);

    NamespaceBuilder()
        .name("user")
        .adoptBy(xi.root());
    Namespace user = xi.root().findNamespace("user");
    BOOST_REQUIRE(user.valid());

    NamespaceBuilder()
        .name("include")
        .parent(user)
        .adoptBy();

    NamespaceBuilder tb;
    tb.parent(user);
    BOOST_CHECK(tb.getParent() == user);

    tb.name("test_name");
    BOOST_CHECK(tb.getName() == "test_name");

    Namespace user_include = user.findNamespace("include");
    BOOST_CHECK(user_include.valid());

    NamespaceBuilder testBuilder;
    
    Type type_int = xi.root().locateType("int", '.');
    BOOST_REQUIRE(type_int.valid());

    TypeBuilder type_b;
    type_b.name("test_type")
        .addMember("m1", "int", type_int)
        .endBuild();
    testBuilder.adopt(type_b);

    NamespaceBuilder ns_b;
    ns_b.name("test_ns");
    testBuilder.adopt(ns_b);

    TypeAliasBuilder alias_b;
    alias_b.name("test_alias")
        .setType(type_int)
        .typeName("int");
    testBuilder.adopt(alias_b);

    ScopedObjectCreator(type_int, xi).adoptBy(testBuilder.get(), "int_obj");

    NamespaceBuilder ns_new;
    ns_new.swap(testBuilder);


    ns_new.adoptChildrenBy(user);

    BOOST_CHECK(user.findNamespace("include").valid());
    BOOST_CHECK(user.findNamespace("test_ns").valid());
    BOOST_CHECK(user.findType("test_type").valid());
    BOOST_CHECK(user.findAlias("test_alias").valid());
    BOOST_CHECK(user.findObject("int_obj").name != 0);

    NamespaceBuilder().createChild("x", '.').adoptChildrenBy(user);

    NamespaceBuilder().createChild("a.b.c", '.').adoptChildrenBy(user);

    Namespace a = user.findNamespace("a");
    BOOST_REQUIRE(a.valid());

    Namespace b = a.findNamespace("b");
    BOOST_REQUIRE(b.valid());

    Namespace c = b.findNamespace("c");
    BOOST_REQUIRE(c.valid());
}
BOOST_AUTO_TEST_SUITE_END()
