cmake_minimum_required(VERSION 3.8)

project(IA DESCRIPTION "Blokus IA core library" LANGUAGES CXX)

# Note that headers are optional, and do not affect add_library, but they will
# not show up in IDEs unless they are listed in add_library.
file(GLOB_RECURSE ia_headers ${CMAKE_SOURCE_DIR}/include/IA/*.h)
file(GLOB_RECURSE ia_sources *.cpp)

add_library(${PROJECT_NAME} ${ia_headers} ${ia_sources})

# We need this directory, and users of our library will need it too
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(IA PRIVATE Core)

# For nicer IDE views
source_group(
  TREE ${CMAKE_SOURCE_DIR}/include/IA
  PREFIX "Header Files"
  FILES ${ia_headers})

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR}
  PREFIX "Source Files"
  FILES ${ia_sources})