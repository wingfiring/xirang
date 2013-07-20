cmake_minimum_required(VERSION 2.6)

#project name
PROJECT(common_test)

INCLUDE_DIRECTORIES(${AIO_ROOT})
INCLUDE_DIRECTORIES(${BOOST_INCLUDE})
LINK_DIRECTORIES(${BOOST_LINK})
#source directory
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/test/common DIR_SRCS)


#add library file
ADD_EXECUTABLE(common_test ${DIR_SRCS})
ENABLE_TESTING()

TARGET_LINK_LIBRARIES(
	common_test
	aio_common
	${ZLIB}
	${Boost_LIBRARIES}
	)

ADD_TEST(common_test common_test)
add_custom_command(
	TARGET common_test
	POST_BUILD COMMAND $<TARGET_FILE:common_test>
	)
