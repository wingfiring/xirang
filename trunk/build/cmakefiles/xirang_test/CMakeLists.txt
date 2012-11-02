cmake_minimum_required(VERSION 2.6)

#project name
PROJECT(xirang_test)

INCLUDE_DIRECTORIES(${AIO_ROOT})
INCLUDE_DIRECTORIES(${BOOST_INCLUDE})
#source directory
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/test/xirang DIR_SRCS)
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/test/xirang/vfs DIR_SRCS)



#add library file
ADD_EXECUTABLE(xirang_test ${DIR_SRCS})
ENABLE_TESTING()
ADD_TEST(xirang_test xirang_test)

TARGET_LINK_LIBRARIES(
	xirang_test
 	xirang
	aio_common
	${ZLIB}
	${BOOST_FILESYSTEM}
	${BOOST_SYSTEM}
	optimized ${BOOST_TEST}
	debug ${BOOST_TEST})
