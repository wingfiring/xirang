/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang/path.h>

BOOST_AUTO_TEST_SUITE(path_suite)
using namespace xirang;

BOOST_AUTO_TEST_CASE(path_ctor_case){
	file_path p;
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
	file_path p1(literal("a/b.c"), pp_none);
	file_path p2(literal("a/b.c"));
	BOOST_CHECK(p1 == p2);

	file_path p3(literal("/a///b.c//"));
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

	BOOST_CHECK(file_path(literal("//")).str() == literal("/"));
	BOOST_CHECK(file_path(literal("//a/b.c//")).str() == literal("//a/b.c"));
	BOOST_CHECK(file_path(literal("///a/b.c//")).str() == literal("/a/b.c"));

	BOOST_CHECK(file_path(literal("///a/b.c/../d")).str() == literal("/a/d"));
	BOOST_CHECK(file_path(literal("/a/b.c/..")).str() == literal("/a"));
	BOOST_CHECK(file_path(literal("/a/b.c/../..")).str() == literal("/"));
	BOOST_CHECK(file_path(literal("/a/b.c/../../..")).str() == literal("/"));
	BOOST_CHECK(file_path(literal("//a/b.c/../../..")).str() == literal("//a"));

	file_path p4(literal("///a/b.c//"), pp_none);
	BOOST_CHECK(!p4.is_normalized());
	p4.normalize();
	BOOST_CHECK(p4.is_normalized());

}
BOOST_AUTO_TEST_CASE(path_parent_case)
{
	file_path p(literal("a/b.c"), pp_none);
	BOOST_CHECK(p.parent().str() == literal("a"));

	BOOST_CHECK(file_path(literal("/a")).parent().str() == literal("/"));
	BOOST_CHECK(file_path(literal("//a")).parent().str() == literal("//a"));
	BOOST_CHECK(file_path(literal("a")).parent().str() == literal(""));
}
BOOST_AUTO_TEST_CASE(path_ext_case)
{
	BOOST_CHECK(file_path(literal("a/b.c")).ext().str() == literal("c"));
	BOOST_CHECK(file_path(literal(".c")).ext().str() == literal("c"));
	BOOST_CHECK(file_path(literal("a.")).ext().str() == literal(""));
	BOOST_CHECK(file_path(literal("a")).ext().str() == literal(""));
}
BOOST_AUTO_TEST_CASE(path_stem_case)
{
	BOOST_CHECK(file_path(literal("a/b.c.d")).stem().str() == literal("b.c"));
	BOOST_CHECK(file_path(literal("a/.c")).stem().str() == literal(""));
	BOOST_CHECK(file_path(literal("//a.b.c")).stem().str() == literal("a.b"));
}
BOOST_AUTO_TEST_CASE(path_filename_case)
{
	BOOST_CHECK(file_path(literal("a/b.c.d")).filename().str() == literal("b.c.d"));
	BOOST_CHECK(file_path(literal("a/.c")).filename().str() == literal(".c"));
	BOOST_CHECK(file_path(literal("//a.b.c")).filename().str() == literal("a.b.c"));
}
BOOST_AUTO_TEST_CASE(path_is_absolute_case)
{
	BOOST_CHECK(file_path(literal("/")).is_absolute());
	BOOST_CHECK(file_path(literal("/")).is_root());
	BOOST_CHECK(!file_path(literal("//")).is_network());
	BOOST_CHECK(file_path(literal("//a")).is_network());
}
BOOST_AUTO_TEST_CASE(path_concat_case)
{
	BOOST_CHECK(file_path(literal("/a"))/file_path(literal("b")) == file_path(literal("/a/b")));
	BOOST_CHECK(file_path(literal("/a"))/file_path(literal("/b")) == file_path(literal("/a/b")));
	BOOST_CHECK(file_path(literal("/a"))/file_path(literal("/")) == file_path(literal("/a")));
	BOOST_CHECK(file_path(literal("a"))/file_path(literal("/")) == file_path(literal("a")));
	BOOST_CHECK(file_path(literal("a"))/file_path(literal("b")) == file_path(literal("a/b")));
	BOOST_CHECK(file_path(literal("a"))/file_path(literal("//b")) == file_path(literal("a/b")));
}
BOOST_AUTO_TEST_CASE(path_is_normalized_case)
{
	BOOST_CHECK(file_path(literal("a/b/../c")).is_normalized());
	BOOST_CHECK(!file_path(literal("a/b/../c"), pp_none).is_normalized());
}

BOOST_AUTO_TEST_CASE(path_is_replace_case)
{
	BOOST_CHECK(file_path(literal("/c:/a/b")).replace_parent(file_path(literal("c"))) == file_path(literal("c/b")));
	BOOST_CHECK(file_path(literal("a/b.c.cpp")).replace_ext(file_path(literal(".hpp"))) == file_path(literal("a/b.c.hpp")));
	BOOST_CHECK(file_path(literal("a/b.c.cpp")).replace_ext(file_path(literal("hpp"))) == file_path(literal("a/b.c.hpp")));
	BOOST_CHECK(file_path(literal("a/b.c.cpp")).replace_stem(file_path(literal("bc"))) == file_path(literal("a/bc.cpp")));
	BOOST_CHECK(file_path(literal("a/b.c.cpp")).replace_filename(file_path(literal("bc.cpp"))) == file_path(literal("a/bc.cpp")));
	BOOST_CHECK(file_path(literal("a/b.c.cpp")).to_absolute() == file_path(literal("/a/b.c.cpp")));
	BOOST_CHECK(file_path(literal("/a/b.c.cpp")).remove_absolute() == file_path(literal("a/b.c.cpp")));
}
BOOST_AUTO_TEST_CASE(path_windows_case)
{
	BOOST_CHECK(file_path(literal("c:/a/b"), pp_localfile) == file_path(literal("/c:/a/b")));
	BOOST_CHECK(file_path(literal("c:/a/b"), pp_localfile).has_disk());
	BOOST_CHECK(!file_path(literal("c/a/b"), pp_localfile).has_disk());

	BOOST_CHECK(!file_path(literal("c:/a/b"), pp_localfile).is_pure_disk());
	BOOST_CHECK(file_path(literal("c:/a"), pp_localfile).parent().is_pure_disk());
}
BOOST_AUTO_TEST_CASE(path_under_contain_case){
	BOOST_CHECK(file_path(literal("c/a")).contains(file_path(literal("c/a/b"))));
	BOOST_CHECK(file_path(literal("c/b/a")).under(file_path(literal("c/b"))));
}

BOOST_AUTO_TEST_CASE(simple_path_ctor_case){
	simple_path p;
	BOOST_CHECK(p.empty());

	BOOST_CHECK(p.parent().empty());
	BOOST_CHECK(p.filename().empty());
	BOOST_CHECK(!p.is_absolute());
	BOOST_CHECK(!p.is_root());
}

BOOST_AUTO_TEST_CASE(simple_path_normalize_case)
{
	simple_path p3(literal(".a...b/c.."));
	auto itr = p3.begin();

	BOOST_CHECK(itr->str() == literal("."));
	++itr;
	BOOST_CHECK(itr->str() == literal("a"));
	++itr;
	BOOST_CHECK(itr->str() == literal("b/c"));
	++itr;
	BOOST_CHECK(itr == p3.end());

	--itr;
	BOOST_CHECK(itr->str() == literal("b/c"));
	--itr;
	BOOST_CHECK(itr->str() == literal("a"));
	--itr;
	BOOST_CHECK(itr->str() == literal("."));
	BOOST_CHECK(itr == p3.begin());

	BOOST_CHECK(simple_path(literal("..")).str() == literal("."));
	BOOST_CHECK(simple_path(literal("..a.b.c..")).str() == literal(".a.b.c"));
	BOOST_CHECK(simple_path(literal("...a.b.c..")).str() == literal(".a.b.c"));
}
BOOST_AUTO_TEST_CASE(simple_path_parent_case)
{
	simple_path p(literal("a.b.c"), pp_none);
	BOOST_CHECK(p.parent().str() == literal("a.b"));

	BOOST_CHECK(simple_path(literal(".a")).parent().str() == literal("."));
	BOOST_CHECK(simple_path(literal("a")).parent().str() == literal(""));
}
BOOST_AUTO_TEST_CASE(simple_path_filename_case)
{
	BOOST_CHECK(simple_path(literal("a.bcd")).filename().str() == literal("bcd"));
	BOOST_CHECK(simple_path(literal("a./c")).filename().str() == literal("/c"));
	BOOST_CHECK(simple_path(literal("..abc")).filename().str() == literal("abc"));
}
BOOST_AUTO_TEST_CASE(simple_path_is_absolute_case)
{
	BOOST_CHECK(simple_path(literal(".")).is_absolute());
	BOOST_CHECK(simple_path(literal(".")).is_root());
}
BOOST_AUTO_TEST_CASE(simple_path_concat_case)
{
	BOOST_CHECK(simple_path(literal(".a"))/simple_path(literal("b")) == simple_path(literal(".a.b")));
	BOOST_CHECK(simple_path(literal(".a"))/simple_path(literal(".b")) == simple_path(literal(".a.b")));
	BOOST_CHECK(simple_path(literal(".a"))/simple_path(literal(".")) == simple_path(literal(".a")));
	BOOST_CHECK(simple_path(literal("a"))/simple_path(literal(".")) == simple_path(literal("a")));
	BOOST_CHECK(simple_path(literal("a"))/simple_path(literal("b")) == simple_path(literal("a.b")));
	BOOST_CHECK(simple_path(literal("a"))/simple_path(literal("..b")) == simple_path(literal("a.b")));
}

BOOST_AUTO_TEST_CASE(simple_path_is_replace_case)
{
	BOOST_CHECK(simple_path(literal(".c:.a.b")).replace_parent(simple_path(literal("c"))) == simple_path(literal("c.b")));
	BOOST_CHECK(simple_path(literal("a.cpp")).replace_filename(simple_path(literal("bc"))) == simple_path(literal("a.bc")));
	BOOST_CHECK(simple_path(literal("a.b")).to_absolute() == simple_path(literal(".a.b")));
	BOOST_CHECK(simple_path(literal(".a.b.c.cpp")).remove_absolute() == simple_path(literal("a.b.c.cpp")));
}
BOOST_AUTO_TEST_CASE(simple_path_under_contain_case){
	BOOST_CHECK(simple_path(literal("c.a")).contains(simple_path(literal("c.a.b"))));
	BOOST_CHECK(simple_path(literal("c.b.a")).under(simple_path(literal("c.b"))));
}

BOOST_AUTO_TEST_SUITE_END()

