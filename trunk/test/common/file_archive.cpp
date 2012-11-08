/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/archive/file_archive.h>
#include <aio/common/fsutility.h>
#include <aio/common/assert.h>
#include <aio/common/string_algo/utf8.h>

//#include <iostream>

#include "iarchive.h"

BOOST_AUTO_TEST_SUITE(file_archive_suite)

using namespace aio;
using namespace aio::archive;

//using archive_suite::ArchiveTester;

BOOST_AUTO_TEST_CASE(file_archive_wchar_case)
{
    //prepare
    aio::string tmp = aio::fs::temp_dir();
    aio::string file_name = tmp + "/\xd0\xa0\xd0\xb0\xd0\xb7\xd0\xbd\xd0\xbe\xd0\xb5";

    const string text="This is file archive UT content. --over--";
    file_write_archive wr(file_name, of_create_or_open);
    BOOST_CHECK(wr.size() == 0);

    wr.write(string_to_c_range(text));
    BOOST_CHECK(wr.size() == text.size());

#ifndef MSVC_COMPILER_
	unlink(file_name.c_str());
#else
    aio::wstring wpath = utf8::decode_string(file_name);
    _wunlink(wpath.c_str());
#endif
    aio::fs::remove(tmp);
}
BOOST_AUTO_TEST_CASE(file_archive)
{
	//prepare

    string temp_path = fs::temp_dir("tfar_");
	string file_name =  temp_path + fs::private_::gen_temp_name("/fa");


	const string text="This is file archive UT content. --over--";
    aio::range<aio::buffer<aio::byte>::const_iterator> ctext = string_to_c_range(text);
	std::size_t len1 = ((text.size() / 3) | 1) - 1;
	std::size_t len2 = len1 / 2;


	//Writer
	{
		BOOST_CHECK_THROW(file_write_archive(file_name, of_open), archive_open_file_failed);

		file_write_archive wr(file_name, of_create_or_open);

		BOOST_CHECK_THROW(file_write_archive(file_name, of_create), archive_create_file_failed);

		BOOST_REQUIRE(wr.writable());

		file_write_archive::const_iterator pos = wr.write(ctext);

		BOOST_CHECK(pos == ctext.end());
		BOOST_CHECK(wr.offset() == text.size());
		BOOST_CHECK(wr.size() == text.size());

		aio::long_size_t off = aio::long_size_t(wr.seek(len1));
		BOOST_REQUIRE(off == len1);
		BOOST_CHECK(wr.offset() == off);

		wr.write(ctext);
		BOOST_CHECK(wr.offset() == off + text.size());
		BOOST_CHECK(wr.size() == off + text.size());
	}
	//Reader
	{
		file_read_archive rd(file_name);

		BOOST_CHECK(rd.offset() == 0);
		BOOST_CHECK(rd.size() == len1 + text.size());
		BOOST_REQUIRE(rd.readable());

		buffer<aio::byte> buf;
		buf.resize(len1);
		file_read_archive::iterator pos = rd.read(to_range(buf));
		BOOST_CHECK(pos = buf.end());
		BOOST_CHECK(rd.offset() == len1);

		rd.seek(len2);
		BOOST_CHECK(rd.offset() == len2);

		buf.resize(text.size() * 2);

		pos = rd.read(to_range(buf));
		BOOST_CHECK(!rd.readable());
		BOOST_CHECK(pos != buf.end());

		rd.seek(len1);
		pos = rd.read(to_range(buf));
		BOOST_CHECK(!std::lexicographical_compare(buf.begin(), pos, (const aio::byte*)text.begin(), (const aio::byte*)text.end()));
		BOOST_CHECK(!std::lexicographical_compare((const aio::byte*)text.begin(), (const aio::byte*)text.end(), buf.begin(), pos));

		//ArchiveTester tester(rd);
		//rd.seek(0);
		//tester.check_reader();
		//tester.check_random();
	}

	//reader & writer
	{
		file_read_write_archive rw(file_name, of_open);

		BOOST_CHECK(rw.offset() == 0);
		BOOST_CHECK(rw.size() == len1 + text.size());
		BOOST_REQUIRE(rw.readable());
		BOOST_REQUIRE(rw.writable());

		string_builder buf;
		buf.resize(len1);
		range<buffer<aio::byte>::iterator> mbuf = string_to_range(buf);

		file_read_write_archive::iterator pos = rw.read(mbuf);
		BOOST_CHECK(pos == mbuf.end());
		BOOST_CHECK(rw.offset() == len1);

		rw.seek(len2);
		BOOST_CHECK(rw.offset() == len2);

		rw.truncate(text.size());
		BOOST_CHECK(rw.size() == text.size());
		BOOST_CHECK(rw.offset() == len2);

		buf.resize(text.size());

		mbuf = string_to_range(buf);
		pos = rw.read(mbuf);
		BOOST_CHECK(!rw.readable());
		BOOST_CHECK(rw.writable());
		BOOST_CHECK(pos != mbuf.end());

		rw.truncate(len2);
		BOOST_CHECK(rw.offset() == len2);
				
		//ArchiveTester tester(rw);
		//tester.check_writer();
		//rw.seek(0);
		//tester.check_reader();
		//tester.check_random();
	}

    aio::fs::recursive_remove(temp_path);
}
BOOST_AUTO_TEST_SUITE_END()

