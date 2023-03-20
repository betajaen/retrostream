cmake_minimum_required(VERSION 3.24)
project(retrostream_sub C)

set(CMAKE_C_STANDARD 11)
find_package(PkgConfig)

# Threads
find_package(Threads)

# czmq
find_path(PC_CZMQ_INCLUDE_DIR
        NAMES czmq.h
        PATHS ${PC_CZMQ_INCLUDE_DIRS}
        )

find_library(PC_CZMQ_LIBRARY
        NAMES zcmq
        PATHS ${PC_CZMQ_LIBRARY_DIRS}
        )

# SDL
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
add_executable(retrostream_sub main.c sub.c video.c sub.c)
target_include_directories(retrostream_sub PUBLIC  ${PC_CZMQ_INCLUDE_DIR})
target_link_libraries(retrostream_sub PUBLIC  SDL3::SDL3  ${ZeroMQ_LIBRARY})
