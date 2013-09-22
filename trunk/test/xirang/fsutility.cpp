/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/
#include "precompile.h"
#include <xirang/fsutility.h>
#include <sys/stat.h>

BOOST_AUTO_TEST_SUITE(tempfile_suite)
using namespace xirang::fs;
using namespace xirang;

BOOST_AUTO_TEST_CASE(tempfile_case)
{
    xirang::string prefix("test");
    xirang::string test_name = fs::private_::gen_temp_name(prefix);

    BOOST_CHECK(prefix.size() < test_name.size());
    BOOST_CHECK(std::equal(prefix.begin(), prefix.end(), test_name.begin()));

    xirang::string tmppath;
    {   
        xirang::io::file file = temp_file(prefix, xirang::io::of_remove_on_close, &tmppath);
        BOOST_CHECK(!tmppath.empty());
        BOOST_CHECK(state(tmppath).state == xirang::fs::st_regular);
    }
    BOOST_CHECK(state(tmppath).state == xirang::fs::st_not_found);

    BOOST_CHECK_THROW(temp_file(literal("test"), literal("path/not/exist")),  xirang::io::create_failed);

    BOOST_CHECK_THROW(temp_dir(literal("test"), literal("path/not/exist")),  xirang::io::create_failed);

    xirang::string tmpfilepath2;
    {
        tmppath  = temp_dir(prefix);
        BOOST_CHECK(!tmppath.empty());
        BOOST_CHECK_NO_THROW(temp_file(literal("test"), tmppath));
        BOOST_CHECK(state(tmppath).state == xirang::fs::st_dir);

        temp_file(literal("test"), tmppath, 0, &tmpfilepath2);
    }
    BOOST_CHECK(state(tmpfilepath2).state == xirang::fs::st_regular);
    xirang::fs::remove(tmpfilepath2);
    BOOST_CHECK(state(tmpfilepath2).state == xirang::fs::st_not_found);

    xirang::fs::remove(tmppath);
    BOOST_CHECK(state(tmppath).state == xirang::fs::st_not_found);

}

BOOST_AUTO_TEST_CASE(recursive_create_dir_case)
{
    xirang::string tmpdir  = temp_dir(literal("tmp"));
    BOOST_CHECK(recursive_create_dir(tmpdir << literal("/a/b/c")) == xirang::fs::er_ok);

    BOOST_CHECK_NO_THROW(recursive_create(tmpdir << literal("/x/y/z"), xirang::io::of_create));
    recursive_remove(tmpdir);
}

BOOST_AUTO_TEST_CASE(append_tail_slash_case)
{
    BOOST_CHECK(append_tail_slash(literal("/a/b")) == literal("/a/b/"));
    BOOST_CHECK(append_tail_slash(literal("/a/b/")) == literal("/a/b/"));
}

BOOST_AUTO_TEST_CASE(remove_tail_slash_case)
{
    BOOST_CHECK(remove_tail_slash(literal("/a/b")) == literal("/a/b"));
    BOOST_CHECK(remove_tail_slash(literal("/a/b/")) == literal("/a/b"));
}

BOOST_AUTO_TEST_CASE(normalize_case)
{
    BOOST_CHECK(normalize(literal("made/everything/simple/..")) == literal("made/everything/"));
    BOOST_CHECK(normalize(literal("made/everything/../simple")) == literal("made/simple"));
    BOOST_CHECK(normalize(literal("made/everything/../simple/")) == literal("made/simple/"));

    BOOST_CHECK(normalize(literal("/made/everything/../simple/")) == literal("/made/simple/"));
    BOOST_CHECK(normalize(literal("//made/everything/../simple/")) == literal("//made/simple/"));
}

BOOST_AUTO_TEST_CASE(is_normalized_case)
{   
    BOOST_CHECK(!is_normalized(literal("made/everything/../simple")));
    BOOST_CHECK(is_normalized(literal("made/simple/")));
}
BOOST_AUTO_TEST_CASE(is_filename_case)
{   
    BOOST_CHECK(!is_filename(literal("made/everything/../simple")));
    BOOST_CHECK(is_filename(literal("simple")));
}

BOOST_AUTO_TEST_CASE(to_aio_native_path_case)
{   
#ifndef MSVC_COMPILER_
    BOOST_CHECK(to_native_path(literal("made/everything/../simple")) == literal("made/everything/../simple"));
#else
    BOOST_CHECK(to_native_path(literal("made/everything/../simple")) == literal("made\\everything\\..\\simple"));
#endif
    BOOST_CHECK(to_xirang_path(literal("made\\everything\\..\\simple")) == literal("made/everything/../simple"));
}

BOOST_AUTO_TEST_CASE(dir_filename_case)
{   
    xirang::string file;
    xirang::string dir = dir_filename(literal("a/b/d"), &file);
    BOOST_CHECK(dir == literal("a/b"));
    BOOST_CHECK(file == literal("d"));

    dir = dir_filename(literal("a/b/d/"), &file);
    BOOST_CHECK(dir == literal("a/b/d"));
    BOOST_CHECK(file.empty());

    dir = dir_filename(literal("/a/b/d/"), &file);
    BOOST_CHECK(dir == literal("/a/b/d"));
    BOOST_CHECK(file.empty());

    BOOST_CHECK(dir_filename(literal("abc"), &file).empty()) ;
    BOOST_CHECK(file == literal("abc"));
}
BOOST_AUTO_TEST_CASE(ext_filename_case)
{   
    xirang::string file;
    xirang::string ext = ext_filename(literal("a.b"), &file);
    BOOST_CHECK(ext == literal("b"));
    BOOST_CHECK(file == literal("a"));

    ext = ext_filename(literal("a.b."), &file);
    BOOST_CHECK(ext.empty());
    BOOST_CHECK(file == literal("a.b"));

    ext = ext_filename(literal("a"), &file);
    BOOST_CHECK(ext.empty());
    BOOST_CHECK(file == literal("a"));

    ext = ext_filename(literal("a."), &file);
    BOOST_CHECK(ext.empty());
    BOOST_CHECK(file == literal("a"));

    ext = ext_filename(literal(".a"), &file);
    BOOST_CHECK(ext == literal("a"));
    BOOST_CHECK(file.empty());
}
BOOST_AUTO_TEST_CASE(recreate_file_case)
{
    xirang::string tmppath;
	{
		xirang::io::file file = temp_file(literal("test"), 0, &tmppath);
		xirang::buffer<xirang::byte> data;
		data.resize(100);
		BOOST_CHECK(file.write(to_range(data)).empty());

		BOOST_CHECK_NO_THROW(xirang::io::file_reader((xirang::string&)tmppath));

		BOOST_CHECK_NO_THROW(xirang::io::file(tmppath, xirang::io::of_open));
	}
	xirang::fs::remove(tmppath);
}
BOOST_AUTO_TEST_SUITE_END()

