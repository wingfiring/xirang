//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_TEST_HELPER
#define AIO_TEST_HELPER

namespace xirang
{
	template<typename T, int ID = 0>
	struct test_helper;
};

#define AIO_ENABLE_TEST template<typename T, int> friend struct ::xirang::test_helper

#endif //end AIO_TEST_HELPER
