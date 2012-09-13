#ifndef AIO_TEST_HELPER
#define AIO_TEST_HELPER

namespace aio
{
	template<typename T, int ID = 0>
	struct test_helper;
};

#define AIO_ENABLE_TEST template<typename T, int> friend struct ::aio::test_helper

#endif //end AIO_TEST_HELPER
