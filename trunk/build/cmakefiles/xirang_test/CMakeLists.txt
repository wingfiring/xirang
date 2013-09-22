cmake_minimum_required(VERSION 2.6)

#project name
PROJECT(xirang_test)

INCLUDE_DIRECTORIES(${AIO_ROOT})
INCLUDE_DIRECTORIES(${BOOST_INCLUDE})
LINK_DIRECTORIES(${BOOST_LINK})
#source directory
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/test/xirang DIR_SRCS)
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/test/xirang/vfs DIR_SRCS)
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/test/xirang/type DIR_SRCS)



#add library file
ADD_EXECUTABLE(xirang_test ${DIR_SRCS})
ENABLE_TESTING()
ADD_TEST(xirang_test xirang_test)

TARGET_LINK_LIBRARIES(
	xirang_test
 	xirang
	aio_common
	${ZLIB}
	${Boost_LIBRARIES}
	)
add_custom_command(
	TARGET xirang_test
	POST_BUILD COMMAND $<TARGET_FILE:xirang_test>
	)
