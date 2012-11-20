/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/
#include "precompile.h"
#include <aio/common/fsutility.h>
#include <sys/stat.h>

BOOST_AUTO_TEST_SUITE(tempfile_suite)
using namespace aio::fs;
using namespace aio;
using aio::byte;

BOOST_AUTO_TEST_CASE(tempfile_case)
{
    aio::string prefix = "test";
    aio::string test_name = fs::private_::gen_temp_name(prefix);

    BOOST_CHECK(prefix.size() < test_name.size());
    BOOST_CHECK(std::equal(prefix.begin(), prefix.end(), test_name.begin()));

    aio::string tmppath;
    {   
        aio::io::file file = temp_file(prefix, aio::io::of_remove_on_close, &tmppath);
        BOOST_CHECK(!tmppath.empty());
        BOOST_CHECK(state(tmppath).state == aio::fs::st_regular);
    }
    BOOST_CHECK(state(tmppath).state == aio::fs::st_not_found);

    BOOST_CHECK_THROW(temp_file("test", "path/not/exist"),  aio::io::create_failed);

    BOOST_CHECK_THROW(temp_dir("test", "path/not/exist"),  aio::io::create_failed);

    aio::string tmpfilepath2;
    {
        tmppath  = temp_dir(prefix);
        BOOST_CHECK(!tmppath.empty());
        BOOST_CHECK_NO_THROW(temp_file("test", tmppath));
        BOOST_CHECK(state(tmppath).state == aio::fs::st_dir);

        temp_file("test", tmppath, 0, &tmpfilepath2);
    }
    BOOST_CHECK(state(tmpfilepath2).state == aio::fs::st_regular);
    aio::fs::remove(tmpfilepath2);
    BOOST_CHECK(state(tmpfilepath2).state == aio::fs::st_not_found);

    aio::fs::remove(tmppath);
    BOOST_CHECK(state(tmppath).state == aio::fs::st_not_found);

}

BOOST_AUTO_TEST_CASE(recursive_create_dir_case)
{
    aio::string tmpdir  = temp_dir("tmp");
    BOOST_CHECK(recursive_create_dir(tmpdir + "/a/b/c") == aio::fs::er_ok);

    BOOST_CHECK_NO_THROW(recursive_create(tmpdir + "/x/y/z", aio::io::of_create));
    recursive_remove(tmpdir);
}

BOOST_AUTO_TEST_CASE(append_tail_slash_case)
{
    BOOST_CHECK(append_tail_slash("/a/b") == "/a/b/");
    BOOST_CHECK(append_tail_slash("/a/b/") == "/a/b/");
}

BOOST_AUTO_TEST_CASE(remove_tail_slash_case)
{
    BOOST_CHECK(remove_tail_slash("/a/b") == "/a/b");
    BOOST_CHECK(remove_tail_slash("/a/b/") == "/a/b");
}

BOOST_AUTO_TEST_CASE(normalize_case)
{
    BOOST_CHECK(normalize("made/everything/simple/..") == "made/everything/");
    BOOST_CHECK(normalize("made/everything/../simple") == "made/simple");
    BOOST_CHECK(normalize("made/everything/../simple/") == "made/simple/");

    BOOST_CHECK(normalize("/made/everything/../simple/") == "/made/simple/");
    BOOST_CHECK(normalize("//made/everything/../simple/") == "//made/simple/");
}

BOOST_AUTO_TEST_CASE(is_normalized_case)
{   
    BOOST_CHECK(!is_normalized("made/everything/../simple"));
    BOOST_CHECK(is_normalized("made/simple/"));
}
BOOST_AUTO_TEST_CASE(is_filename_case)
{   
    BOOST_CHECK(!is_filename("made/everything/../simple"));
    BOOST_CHECK(is_filename("simple"));
}

BOOST_AUTO_TEST_CASE(to_aio_native_path_case)
{   
#ifndef MSVC_COMPILER_
    BOOST_CHECK(to_native_path("made/everything/../simple") == "made/everything/../simple");
#else
    BOOST_CHECK(to_native_path("made/everything/../simple") == "made\\everything\\..\\simple");
#endif
    BOOST_CHECK(to_aio_path("made\\everything\\..\\simple") == "made/everything/../simple");
}

BOOST_AUTO_TEST_CASE(dir_filename_case)
{   
    aio::string file;
    aio::string dir = dir_filename("a/b/d", &file);
    BOOST_CHECK(dir == "a/b");
    BOOST_CHECK(file == "d");

    dir = dir_filename("a/b/d/", &file);
    BOOST_CHECK(dir == "a/b/d");
    BOOST_CHECK(file.empty());

    dir = dir_filename("/a/b/d/", &file);
    BOOST_CHECK(dir == "/a/b/d");
    BOOST_CHECK(file.empty());

    BOOST_CHECK(dir_filename("abc", &file).empty()) ;
    BOOST_CHECK(file == "abc");
}
BOOST_AUTO_TEST_CASE(ext_filename_case)
{   
    aio::string file;
    aio::string ext = ext_filename("a.b", &file);
    BOOST_CHECK(ext == "b");
    BOOST_CHECK(file == "a");

    ext = ext_filename("a.b.", &file);
    BOOST_CHECK(ext.empty());
    BOOST_CHECK(file == "a.b");

    ext = ext_filename("a", &file);
    BOOST_CHECK(ext.empty());
    BOOST_CHECK(file == "a");

    ext = ext_filename("a.", &file);
    BOOST_CHECK(ext.empty());
    BOOST_CHECK(file == "a");

    ext = ext_filename(".a", &file);
    BOOST_CHECK(ext == "a");
    BOOST_CHECK(file.empty());
}
BOOST_AUTO_TEST_CASE(recreate_file_case)
{
    aio::string tmppath;
	{
		aio::io::file file = temp_file("test", 0, &tmppath);
		aio::buffer<aio::byte> data;
		data.resize(100);
		BOOST_CHECK(file.write(to_range(data)).empty());

		BOOST_CHECK_NO_THROW(aio::io::file_reader((aio::string&)tmppath));

		BOOST_CHECK_NO_THROW(aio::io::file(tmppath, aio::io::of_open));
	}
	aio::fs::remove(tmppath);
}
BOOST_AUTO_TEST_SUITE_END()

