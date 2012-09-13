cmake_minimum_required(VERSION 2.6)

#project name
PROJECT(common_test)

INCLUDE_DIRECTORIES(${AIO_ROOT})
INCLUDE_DIRECTORIES(${BOOST_INCLUDE})
#source directory
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/test/common DIR_SRCS)


#add library file
ADD_EXECUTABLE(common_test ${DIR_SRCS})
ENABLE_TESTING()
ADD_TEST(common_test common_test)

TARGET_LINK_LIBRARIES(
	common_test
	aio_common
	${BOOST_FILESYSTEM}
	optimized ${BOOST_TEST}
	debug ${BOOST_TEST})
