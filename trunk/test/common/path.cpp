/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/path.h>

BOOST_AUTO_TEST_SUITE(path_suite)
using namespace aio;

BOOST_AUTO_TEST_CASE(path_ctor_case){
	path p;
	BOOST_CHECK(p.empty());

	BOOST_CHECK(p.parent().empty());
	BOOST_CHECK(p.ext().empty());
	BOOST_CHECK(p.stem().empty());
	BOOST_CHECK(p.filename().empty());
	BOOST_CHECK(!p.is_absolute());
	BOOST_CHECK(!p.is_network());
	BOOST_CHECK(!p.is_root());
	BOOST_CHECK(p.is_normalized());
}

BOOST_AUTO_TEST_CASE(path_normalize_case)
{
	path p1(literal("a/b.c"), pp_none);
	path p2(literal("a/b.c"));
	BOOST_CHECK(p1 == p2);

	path p3(literal("/a///b.c//"));
	auto itr = p3.begin();

	BOOST_CHECK(itr->str() == literal("/"));
	++itr;
	BOOST_CHECK(itr->str() == literal("a"));
	++itr;
	BOOST_CHECK(itr->str() == literal("b.c"));
	++itr;
	BOOST_CHECK(itr == p3.end());

	--itr;
	BOOST_CHECK(itr->str() == literal("b.c"));
	--itr;
	BOOST_CHECK(itr->str() == literal("a"));
	--itr;
	BOOST_CHECK(itr->str() == literal("/"));
	BOOST_CHECK(itr == p3.begin());

	BOOST_CHECK(path(literal("//")).str() == literal("/"));
	BOOST_CHECK(path(literal("//a/b.c//")).str() == literal("//a/b.c"));
	BOOST_CHECK(path(literal("///a/b.c//")).str() == literal("/a/b.c"));

	BOOST_CHECK(path(literal("///a/b.c/../d")).str() == literal("/a/d"));
	BOOST_CHECK(path(literal("/a/b.c/..")).str() == literal("/a"));
	BOOST_CHECK(path(literal("/a/b.c/../..")).str() == literal("/"));
	BOOST_CHECK(path(literal("/a/b.c/../../..")).str() == literal("/"));
	BOOST_CHECK(path(literal("//a/b.c/../../..")).str() == literal("//a"));

	path p4(literal("///a/b.c//"), pp_none);
	BOOST_CHECK(!p4.is_normalized());
	p4.normalize();
	BOOST_CHECK(p4.is_normalized());

}
BOOST_AUTO_TEST_CASE(path_parent_case)
{
	path p(literal("a/b.c"), pp_none);
	BOOST_CHECK(p.parent().str() == literal("a"));

	BOOST_CHECK(path(literal("/a")).parent().str() == literal("/"));
	BOOST_CHECK(path(literal("//a")).parent().str() == literal("//a"));
	BOOST_CHECK(path(literal("a")).parent().str() == literal(""));
}
BOOST_AUTO_TEST_CASE(path_ext_case)
{
	BOOST_CHECK(path(literal("a/b.c")).ext().str() == literal("c"));
	BOOST_CHECK(path(literal(".c")).ext().str() == literal("c"));
	BOOST_CHECK(path(literal("a.")).ext().str() == literal(""));
	BOOST_CHECK(path(literal("a")).ext().str() == literal(""));
}
BOOST_AUTO_TEST_CASE(path_stem_case)
{
	BOOST_CHECK(path(literal("a/b.c.d")).stem().str() == literal("b.c"));
	BOOST_CHECK(path(literal("a/.c")).stem().str() == literal(""));
	BOOST_CHECK(path(literal("//a.b.c")).stem().str() == literal("a.b"));
}
BOOST_AUTO_TEST_CASE(path_filename_case)
{
	BOOST_CHECK(path(literal("a/b.c.d")).filename().str() == literal("b.c.d"));
	BOOST_CHECK(path(literal("a/.c")).filename().str() == literal(".c"));
	BOOST_CHECK(path(literal("//a.b.c")).filename().str() == literal("a.b.c"));
}
BOOST_AUTO_TEST_CASE(path_is_absolute_case)
{
	BOOST_CHECK(path(literal("/")).is_absolute());
	BOOST_CHECK(path(literal("/")).is_root());
	BOOST_CHECK(!path(literal("//")).is_network());
	BOOST_CHECK(path(literal("//a")).is_network());
}
BOOST_AUTO_TEST_CASE(path_concat_case)
{
	BOOST_CHECK(path(literal("/a"))/path(literal("b")) == path(literal("/a/b")));
	BOOST_CHECK(path(literal("/a"))/path(literal("/b")) == path(literal("/a/b")));
	BOOST_CHECK(path(literal("/a"))/path(literal("/")) == path(literal("/a")));
	BOOST_CHECK(path(literal("a"))/path(literal("/")) == path(literal("a")));
	BOOST_CHECK(path(literal("a"))/path(literal("b")) == path(literal("a/b")));
	BOOST_CHECK(path(literal("a"))/path(literal("//b")) == path(literal("a/b")));
}

BOOST_AUTO_TEST_SUITE_END()

