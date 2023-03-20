cmake_minimum_required(VERSION 3.6)
project(retrostream_pub C)

find_package(PkgConfig)
pkg_check_modules(PC_CZMQ QUIET czmq)
find_path(PC_CZMQ_INCLUDE_DIR
        NAMES czmq.h
        PATHS ${PC_CZMQ_INCLUDE_DIRS}
        )

find_library(PC_CZMQ_LIBRARY
        NAMES zcmq
        PATHS ${PC_CZMQ_LIBRARY_DIRS}
        )

add_executable(retrostream_pub main.c video.c pub.c)
target_include_directories(retrostream_pub PUBLIC ${PC_CZMQ_INCLUDE_DIR})
target_link_libraries(retrostream_pub PUBLIC ${ZeroMQ_LIBRARY})
