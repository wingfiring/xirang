#ifndef AIO_COMMON_TEST_IARCHIVE_H
#define AIO_COMMON_TEST_IARCHIVE_H
#include <aio/common/iarchive.h>
#include <aio/common/interface.h>

BOOST_AUTO_TEST_SUITE(archive_suite)

using namespace aio;

class ArchiveTester
{
	public:
	ArchiveTester& check_reader(interface_ref<io::reader>);
	ArchiveTester& check_writer(interface_ref<io::writer>);
	ArchiveTester& check_sequence(interface_ref<io::sequence>);
	ArchiveTester& check_forward(interface_ref<io::forward>);
	ArchiveTester& check_random(interface_ref<io::random>);
	ArchiveTester& check_reader_random(interface_ref<io::reader, io::random> ar);
	ArchiveTester& check_writer_random(interface_ref<io::writer, io::random> ar);

	ArchiveTester& check_rd_view(interface_ref<io::read_view>);
	ArchiveTester& check_wr_view(interface_ref<io::write_view>);
};
BOOST_AUTO_TEST_SUITE_END()

#endif //end AIO_COMMON_TEST_IARCHIVE_H
