cmake_minimum_required(VERSION 3.8)

project(Core DESCRIPTION "Core library" LANGUAGES CXX)

# Note that headers are optional, and do not affect add_library, but they will
# not show up in IDEs unless they are listed in add_library.
file(GLOB_RECURSE core_headers ${CMAKE_SOURCE_DIR}/include/Core/*.h)
file(GLOB_RECURSE core_sources *.cpp)

add_library(${PROJECT_NAME} ${core_headers} ${core_sources})

# We need this directory, and users of our library will need it too
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Enabled warnings
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

# For nicer IDE views
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")

source_group(
  TREE ${CMAKE_SOURCE_DIR}/include/Core
  PREFIX "Header Files"
  FILES ${core_headers})

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR}
  PREFIX "Source Files"
  FILES ${core_sources})