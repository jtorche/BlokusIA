cmake_minimum_required(VERSION 3.0)

project(BlokusIA)
add_definitions(-DNOMINMAX)

#Core
file(GLOB_RECURSE CORE_HDRS src/timCore/*.h)
#file(GLOB_RECURSE CORE_HDRS src/timCore/*.hpp)
file(GLOB_RECURSE CORE_SRCS src/timCore/*.cpp)
ADD_LIBRARY(timCore ${CORE_SRCS} ${CORE_HDRS})
target_include_directories(timCore PRIVATE "src/")
set_property(TARGET timCore PROPERTY CXX_STANDARD 17)

# Main exe
file(GLOB HDRS src/*.h)
file(GLOB SRCS src/*.cpp)

# Unit test exe
file(GLOB TestHDRS src/test/*.h)
file(GLOB TestSRCS src/test/*.cpp)

ADD_EXECUTABLE(Program ${HDRS} ${SRCS})
target_include_directories(Program PRIVATE "src/")
target_link_libraries(Program timCore)
set_property(TARGET Program PROPERTY CXX_STANDARD 17)