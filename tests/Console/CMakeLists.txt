cmake_minimum_required(VERSION 3.16)

project(Console_Tests DESCRIPTION "Console application to easily test Blokus AI" LANGUAGES CXX)

if (BUILD_TESTING)
    file(GLOB_RECURSE console_tests_headers *.h)
    file(GLOB_RECURSE console_tests_sources *.cpp)
    set(console_tests_precompile_header "${CMAKE_CURRENT_SOURCE_DIR}/precompile/precompile_console_tests.h")

    add_executable(${PROJECT_NAME} ${console_tests_headers} ${console_tests_sources} ${console_tests_precompile_header})

    # Enabled warnings
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

    # Precompile headers
    target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${console_tests_precompile_header}>")

    # Linked libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE Core AI)

    # For nicer IDE views
    source_group(
      TREE ${CMAKE_CURRENT_SOURCE_DIR}
      PREFIX "Header Files"
      FILES ${console_tests_headers})

    source_group(
      TREE ${CMAKE_CURRENT_SOURCE_DIR}
      PREFIX "Source Files"
      FILES ${console_tests_sources})

    source_group(
      "Precompile Header File"
      FILES ${console_tests_precompile_header})
endif()