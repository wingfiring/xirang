/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/
#include "precompile.h"
#include <aio/common/fsutility.h>
#include <sys/stat.h>

BOOST_AUTO_TEST_SUITE(tempfile_suite)
using namespace aio::fs;

BOOST_AUTO_TEST_CASE(tempfile_case)
{
    aio::string prefix = "test";
    aio::string test_name = private_::gen_temp_name(prefix);

    BOOST_CHECK(prefix.size() < test_name.size());
    BOOST_CHECK(std::equal(prefix.begin(), prefix.end(), test_name.begin()));

    aio::string tmppath;
    {   
        aio::archive::archive_ptr file = temp_file(prefix, aio::archive::of_remove_on_close, &tmppath).move();
        BOOST_CHECK(file);
        BOOST_CHECK(!tmppath.empty());
        BOOST_CHECK(state(tmppath).state == aio::fs::st_regular);
    }
    BOOST_CHECK(state(tmppath).state == aio::fs::st_not_found);

    BOOST_CHECK_THROW(temp_file("test", "path/not/exist"),  aio::archive::create_failed);

    BOOST_CHECK_THROW(temp_dir("test", "path/not/exist"),  aio::archive::create_failed);

    aio::string tmpfilepath2;
    {
        tmppath  = temp_dir(prefix);
        BOOST_CHECK(!tmppath.empty());
        BOOST_CHECK(temp_file("test", tmppath).move());
        BOOST_CHECK(state(tmppath).state == aio::fs::st_dir);

        temp_file("test", tmppath, 0, &tmpfilepath2);
    }
    BOOST_CHECK(state(tmpfilepath2).state == aio::fs::st_regular);
    aio::fs::remove(tmpfilepath2);
    BOOST_CHECK(state(tmpfilepath2).state == aio::fs::st_not_found);

    aio::fs::remove(tmppath);
    BOOST_CHECK(state(tmppath).state == aio::fs::st_not_found);

}

BOOST_AUTO_TEST_CASE(create_case)
{
    BOOST_CHECK(!create("path/not/found", aio::archive::mt_write | aio::archive::mt_random, aio::archive::of_create_or_open));

    aio::string tmpdir  = temp_dir("");
    aio::string tmpfile = tmpdir + "/file1";

    BOOST_CHECK(!create(tmpfile, aio::archive::mt_write | aio::archive::mt_random, aio::archive::of_open));

    BOOST_CHECK(create(tmpfile, aio::archive::mt_write | aio::archive::mt_random, aio::archive::of_create).move());

    aio::archive::archive_ptr file = create(tmpfile, aio::archive::mt_write | aio::archive::mt_random, aio::archive::of_create_or_open).move();
    BOOST_CHECK(file);
    BOOST_CHECK(file->query_writer());
    BOOST_CHECK(file->query_random());

    aio::string tmpfile2 = tmpdir + "/file2";
    const aio::byte txt[] = "quick fox jump over the lazy dog";
    aio::buffer<aio::byte> buf (aio::make_range(txt, txt + sizeof(txt)-1));
    file->query_writer()->write(to_range(buf));
    file.reset();

    aio::fs::copy(tmpfile, tmpfile2);
    aio::archive::archive_ptr file2 = create(tmpfile2, aio::archive::mt_read | aio::archive::mt_random, aio::archive::of_open).move();
    BOOST_REQUIRE(file2);

    aio::buffer<aio::byte> buf2;
    buf2.resize(buf.size());
    file2->query_reader()->read(aio::to_range(buf2));
    file2.reset();
    BOOST_CHECK(std::equal(buf.begin(), buf.end(), buf2.begin()));

    BOOST_CHECK(aio::fs::truncate(tmpfile2, 5) == aio::fs::er_ok);
    BOOST_CHECK(aio::fs::state(tmpfile2).size == 5);

    aio::fs::file_range filelist = children(tmpdir);
    aio::fs::file_range::iterator file_itr = filelist.begin();
    BOOST_CHECK(file_itr != filelist.end());
    aio::string filepath = aio::fs::append_tail_slash(tmpdir) + *file_itr;
    BOOST_CHECK(filepath == tmpfile || filepath == tmpfile2);
    ++file_itr;
    filepath = aio::fs::append_tail_slash(tmpdir) + *file_itr;
    BOOST_CHECK(filepath == tmpfile || filepath == tmpfile2);
    ++file_itr;
    BOOST_CHECK(file_itr == filelist.end());

    BOOST_CHECK(exists(tmpfile));
    BOOST_CHECK(!exists(tmpfile + "/notfound"));


    recursive_remove(tmpdir);

    BOOST_CHECK(state(tmpdir).state == aio::fs::st_not_found);
}
BOOST_AUTO_TEST_CASE(recursive_create_dir_case)
{
    aio::string tmpdir  = temp_dir("tmp");
    BOOST_CHECK(recursive_create_dir(tmpdir + "/a/b/c") == aio::fs::er_ok);

    BOOST_CHECK(recursive_create(tmpdir + "/x/y/z", aio::archive::mt_write | aio::archive::mt_random, aio::archive::of_create).move());
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
    aio::archive::archive_ptr file = temp_file("test", 0, &tmppath).move();
    aio::buffer<aio::byte> data;
    data.resize(100);
    file->query_writer()->write(to_range(data));

    aio::archive::archive_ptr file2 = create(tmppath, aio::archive::mt_read |aio::archive::mt_random, aio::archive::of_open).move();
    BOOST_CHECK(file2);
    BOOST_CHECK(file2->query_writer() == 0);

    file.reset();
    aio::archive::archive_ptr file3 = create(tmppath, aio::archive::mt_write | aio::archive::mt_read |aio::archive::mt_random, aio::archive::of_open).move();
    BOOST_CHECK(file3);
    BOOST_CHECK(file3->query_writer() );

    aio::fs::remove(tmppath);
}
BOOST_AUTO_TEST_SUITE_END()

