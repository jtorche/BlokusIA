cmake_minimum_required(VERSION 3.0)

project(BlokusIA)
add_definitions(-DNOMINMAX)

# Core
file(GLOB_RECURSE CORE_HDRS src/Core/*.h)
file(GLOB_RECURSE CORE_SRCS src/Core/*.cpp)

ADD_LIBRARY(Core ${CORE_SRCS} ${CORE_HDRS})
target_include_directories(Core PRIVATE "src/")
set_property(TARGET Core PROPERTY CXX_STANDARD 17)


# IA exe
file(GLOB_RECURSE IA_HDRS src/IA/*.h)
file(GLOB_RECURSE IA_SRCS src/IA/*.cpp)

ADD_EXECUTABLE(IA ${IA_HDRS} ${IA_SRCS})
target_include_directories(IA PRIVATE "src/")
target_link_libraries(IA Core)
set_property(TARGET IA PROPERTY CXX_STANDARD 17)