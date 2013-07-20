cmake_minimum_required(VERSION 2.6)

#project name
PROJECT(utility_ext1)

INCLUDE_DIRECTORIES(${AIO_ROOT})
INCLUDE_DIRECTORIES(${BOOST_INCLUDE})
LINK_DIRECTORIES(${BOOST_LINK})
#source directory
AUX_SOURCE_DIRECTORY(${AIO_ROOT}/src/utility/ext1 DIR_SRCS)

#add library file
ADD_LIBRARY(utility_ext1 STATIC ${DIR_SRCS})


