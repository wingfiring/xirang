#include "../precompile.h"
#include <aio/common/fsutility.h>
#include <aio/xirang/vfs.h>

BOOST_AUTO_TEST_SUITE(vfs_suite)
using namespace xirang::fs;
using namespace xirang;
using aio::io::archive_mode;
using aio::io::open_flag;

// TODO: move them into aio
BOOST_AUTO_TEST_CASE(utility_case)
{
	string path = "/test";
	string expect = "/test/";
	
	BOOST_CHECK(append_tail_slash(path) == expect);
	BOOST_CHECK(append_tail_slash(expect) == expect);

	string empty;
	BOOST_CHECK(append_tail_slash(empty) == string("/"));


	BOOST_CHECK(!is_absolute(empty));
	BOOST_CHECK(is_absolute(path));
	BOOST_CHECK(!is_absolute(string("test")));

	BOOST_CHECK(!is_normalized(string(".")));
	BOOST_CHECK(!is_normalized(string("..")));

	BOOST_CHECK(normalize("") == "");

	string long_path = "made/every/thing/./simple/../..";
	string long_path_exp = "made/every/";
	BOOST_CHECK(long_path_exp == normalize(long_path));
	BOOST_CHECK(!is_normalized(long_path));
	BOOST_CHECK(is_normalized(long_path_exp));

	string long_path_0 = "made/every/thing/./simple/../../";
	BOOST_CHECK(long_path_exp == normalize(long_path_0));


	string long_path1 = "made/every/thing/./simple/../../as";
	string long_path_exp1 = "made/every/as";
	BOOST_CHECK(long_path_exp1 == normalize(long_path1));

	string long_path2 = "made/every/thing/./simple/../../as/";
	string long_path_exp2 = "made/every/as/";
	BOOST_CHECK(long_path_exp2 == normalize(long_path2));

	string long_path3 = "made/every/thing/./simple/../../../../../../";
	string long_path_exp3 = "";
	BOOST_CHECK(long_path_exp3 == normalize(long_path3));

	string long_path4 = "/made/every/thing/./simple/../../../../../../";
	string long_path_exp4 = "/";
	BOOST_CHECK(long_path_exp4 == normalize(long_path4));
}

/// TODO: add cases for other vfs based utilities

BOOST_AUTO_TEST_SUITE_END()
