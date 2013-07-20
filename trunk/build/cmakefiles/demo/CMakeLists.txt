cmake_minimum_required(VERSION 2.6)

#project name
PROJECT(demo)

INCLUDE_DIRECTORIES(${AIO_ROOT})
INCLUDE_DIRECTORIES(${BOOST_INCLUDE})
#source directory
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/src/demo DIR_SRCS)



#add library file
ADD_EXECUTABLE(demo ${DIR_SRCS})

TARGET_LINK_LIBRARIES(
	demo
 	xirang
	aio_common
	${BOOST_FILESYSTEM}
	${BOOST_SYSTEM}
	${ZLIB}
)
