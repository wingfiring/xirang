cmake_minimum_required(VERSION 2.6)

#project name
PROJECT(utility_ext1_test)

INCLUDE_DIRECTORIES(${AIO_ROOT})
INCLUDE_DIRECTORIES(${BOOST_INCLUDE})
LINK_DIRECTORIES(${BOOST_LINK})
#source directory
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/test/utility/ext1 DIR_SRCS)



#add library file
ADD_EXECUTABLE(utility1_test ${DIR_SRCS})
ENABLE_TESTING()
ADD_TEST(utility1_test utility1_test)

TARGET_LINK_LIBRARIES(
	utility1_test
 	utility_ext1
	aio_common
	${Boost_LIBRARIES}
	)
add_custom_command(
	TARGET utility1_test
	POST_BUILD COMMAND $<TARGET_FILE:utility1_test>
	)
