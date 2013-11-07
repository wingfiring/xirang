/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang/io.h>
#include <xirang/io/memory.h>
#include <xirang/deflate.h>

BOOST_AUTO_TEST_SUITE(deflate_suite)
	using namespace xirang;

BOOST_AUTO_TEST_CASE(deflate_io_case){
	string text("quick fox jump over the lazy dog");
	auto first = (const byte*)&*text.begin();
	auto src = make_range(first, first + text.size());

	io::mem_archive dest1;
	iref<io::write_map> zip_dest(dest1);

	zip::deflate_writer deflater(zip_dest.get<io::write_map>());
	BOOST_CHECK(deflater.valid());
	BOOST_CHECK(deflater.writable());
	BOOST_CHECK(deflater.offset() == 0);
	BOOST_CHECK(deflater.size() == 0);
	BOOST_CHECK(deflater.uncompressed_size() == 0);
	BOOST_CHECK(!deflater.finished());

	auto ret = deflater.write(src);
	BOOST_CHECK(ret.empty());
	deflater.finish();
	BOOST_CHECK(deflater.finished());
	BOOST_CHECK(dest1.size() >= deflater.size());
	dest1.truncate(deflater.size());
	BOOST_CHECK(dest1.size() == deflater.size());
	BOOST_CHECK(deflater.uncompressed_size() == text.size());

	iref<io::read_map> zip_src(dest1);
	zip::inflate_reader inflater(zip_src.get<io::read_map>());
	BOOST_CHECK(inflater.valid());
	BOOST_CHECK(inflater.readable());
	BOOST_CHECK(inflater.size() == long_size_t(-1));

	io::mem_archive inflated;
	io::copy_data<io::reader, io::writer>(inflater, inflated);
	BOOST_CHECK(std::equal(inflated.data().begin(), inflated.data().end(), src.begin()));
	BOOST_CHECK(!inflater.readable());
	BOOST_CHECK(inflater.compressed_size() == dest1.size());
}
BOOST_AUTO_TEST_SUITE_END()


