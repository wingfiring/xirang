#ifndef AIO_COMMON_TEST_IARCHIVE_H
#define AIO_COMMON_TEST_IARCHIVE_H
#include <aio/common/iarchive.h>
BOOST_AUTO_TEST_SUITE(archive_suite)

using namespace aio;

class ArchiveTester
{
	public:
	explicit ArchiveTester(aio::archive::iarchive& ar);
	ArchiveTester& check_reader();
	ArchiveTester& check_writer();
	ArchiveTester& check_sequence();
	ArchiveTester& check_forward();
	ArchiveTester& check_random();
	
	private:
	aio::archive::iarchive* m_ar;
};
BOOST_AUTO_TEST_SUITE_END()

#endif //end AIO_COMMON_TEST_IARCHIVE_H
