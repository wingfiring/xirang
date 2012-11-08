/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/archive/mem_archive.h>
#include "iarchive.h"

BOOST_AUTO_TEST_SUITE(archive_suite)
using namespace aio;

BOOST_AUTO_TEST_CASE(buffer_in_case)
{
	aio::buffer<aio::byte> buf(128, byte('X'));

	archive_new::buffer_in ar(buf);
	ArchiveTester tester;

	tester.check_reader(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(buffer_out_case)
{
	aio::buffer<aio::byte> buf(128, byte('X'));

	archive_new::buffer_out ar(buf);
	ArchiveTester tester;

	tester.check_writer(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(buffer_io_case)
{
	aio::buffer<aio::byte> buf(128, byte('X'));

	archive_new::buffer_io ar(buf);
	ArchiveTester tester;

	tester.check_reader(ar);
	tester.check_writer(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(mem_read_archive_case)
{
	aio::buffer<aio::byte> buf(128, byte('X'));

	archive_new::mem_read_archive ar(buf);
	ArchiveTester tester;

	tester.check_reader(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(mem_write_archive_case)
{
	archive_new::mem_write_archive ar;
	ArchiveTester tester;

	tester.check_writer(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(mem_read_write_archive_case)
{
	archive_new::mem_read_write_archive ar;
	ArchiveTester tester;

	tester.check_writer(ar);
	ar.seek(0);
	tester.check_reader(ar);
	tester.check_random(ar);
}


BOOST_AUTO_TEST_CASE(mem_archive_case)
{
	archive_new::mem_read_write_archive ar;
	interface_ref<archive_new::reader, archive_new::writer> iar(ar);

	archive_new::writer& wr = iar.get<archive_new::writer>();
	archive_new::reader& rd = iar.get<archive_new::reader>();
	using namespace lio;

	int i = 3;
	wr & i;

	BOOST_CHECK(ar.size() == sizeof(int));

	ar.seek(0);
	BOOST_CHECK(ar.offset() == 0);
	int j;
	rd & j;
	BOOST_CHECK(j == 3);

	ar.truncate(sizeof(int)*2);
	BOOST_CHECK(ar.size() == sizeof(int)*2);

	ar.truncate(0);
	BOOST_CHECK(ar.size() == 0);

}

BOOST_AUTO_TEST_SUITE_END()
