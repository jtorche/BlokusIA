cmake_minimum_required(VERSION 3.16)

project(AI DESCRIPTION "Blokus AI core library" LANGUAGES CXX)

# Note that headers are optional, and do not affect add_library, but they will
# not show up in IDEs unless they are listed in add_library.

if(ENABLE_BLOCKUS_PROJECT)
	file(GLOB_RECURSE ai_headers ${CMAKE_SOURCE_DIR}/include/AI/blockusAI/*.h)
	file(GLOB_RECURSE ai_sources blockusAI/*.cpp)
endif()

if(ENABLE_7W_PROJECT)
	file(GLOB_RECURSE ai_headers ${CMAKE_SOURCE_DIR}/include/AI/7WDuel/*.h)
	file(GLOB_RECURSE ai_sources 7WDuel/*.cpp)
endif()

set(ai_precompile_header "${CMAKE_CURRENT_SOURCE_DIR}/precompile/precompile_ai.h")

add_library(${PROJECT_NAME} ${ai_headers} ${ai_sources} ${ai_precompile_header})

# We need this directory, and users of our library will need it too
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Enabled warnings
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

# Precompile headers
target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ai_precompile_header}>")

# Linked libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Core)

# For nicer IDE views
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")

source_group(
  TREE ${CMAKE_SOURCE_DIR}/include/AI
  PREFIX "Header Files"
  FILES ${ai_headers})

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR}
  PREFIX "Source Files"
  FILES ${ai_sources})

source_group(
  "Precompile Header File"
  FILES ${ai_precompile_header})