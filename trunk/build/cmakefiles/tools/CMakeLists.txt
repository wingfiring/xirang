cmake_minimum_required(VERSION 2.6)

#project name
PROJECT(tools)

INCLUDE_DIRECTORIES(${AIO_ROOT})
INCLUDE_DIRECTORIES(${BOOST_INCLUDE})
LINK_DIRECTORIES(${BOOST_LINK})

define_tools(gentypever)

define_tools(zipdoc)
TARGET_LINK_LIBRARIES(zipdoc fcgi fcgi++)

define_tools(lvvfs)
