/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang/io.h>
#include <xirang/io/memory.h>

BOOST_AUTO_TEST_SUITE(io_suite)
	using namespace xirang;

BOOST_AUTO_TEST_CASE(copy_data_case){
	string text("quick fox jump over the lazy dog");
	auto first = (const byte*)&*text.begin();
	io::buffer_in src(make_range(first, first + text.size()));
	io::mem_archive dest1;

	auto copied_size_1 = io::copy_data<io::reader, io::writer>(src, dest1);
	BOOST_CHECK(copied_size_1 == text.size());
	BOOST_CHECK(std::equal(first, first + text.size(), dest1.data().begin()));
	BOOST_CHECK(dest1.data().size() == text.size());

	src.seek(0);
	dest1.truncate(0);

	auto copied_size_2 = io::copy_data<io::reader, io::write_map>(src, dest1);
	BOOST_CHECK(copied_size_2 == text.size());
	BOOST_CHECK(std::equal(first, first + text.size(), dest1.data().begin()));
	BOOST_CHECK(dest1.data().size() >= text.size());

	src.seek(0);
	dest1.truncate(0);
	auto copied_size_3 = io::copy_data<io::read_map, io::writer>(src, dest1);
	BOOST_CHECK(copied_size_3 == text.size());
	BOOST_CHECK(std::equal(first, first + text.size(), dest1.data().begin()));
	BOOST_CHECK(dest1.data().size() >= text.size());

	src.seek(0);
	dest1.truncate(0);
	auto copied_size_4 = io::copy_data<io::read_map, io::write_map>(src, dest1);
	BOOST_CHECK(copied_size_4 == text.size());
	BOOST_CHECK(std::equal(first, first + text.size(), dest1.data().begin()));
	BOOST_CHECK(dest1.data().size() == text.size());

}
BOOST_AUTO_TEST_SUITE_END()

