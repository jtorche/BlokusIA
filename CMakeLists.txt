cmake_minimum_required(VERSION 3.0)

project(BlokusIA VERSION 1.0.0 DESCRIPTION "Blokus Artificial Intelligence" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DNOMINMAX)

# Core
file(GLOB_RECURSE CORE_HDRS src/Core/*.h)
file(GLOB_RECURSE CORE_SRCS src/Core/*.cpp)

add_library(Core ${CORE_HDRS} ${CORE_SRCS})


# IA Lib
file(GLOB_RECURSE IA_HDRS src/IA/*.h)
file(GLOB_RECURSE IA_SRCS src/IA/*.cpp)

add_library(IA ${IA_HDRS} ${IA_SRCS})
target_include_directories(IA PUBLIC "src/")
target_link_libraries(IA PRIVATE Core)


# IA Tests exe
file(GLOB_RECURSE Test_IA_HDRS src/Tests/*.h)
file(GLOB_RECURSE Test_IA_SRCS src/Tests/*.cpp)

add_executable(Tests_IA ${Test_IA_HDRS} ${Test_IA_SRCS})
target_link_libraries(Tests_IA PRIVATE Core IA)


# UI exe
add_subdirectory(src/UI)