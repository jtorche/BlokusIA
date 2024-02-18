cmake_minimum_required(VERSION 3.16)

project(7W_Tests DESCRIPTION "Console application to easily test 7W AI" LANGUAGES CXX)

if (BUILD_TESTING)
    file(GLOB_RECURSE 7W_tests_headers *.h)
    file(GLOB_RECURSE 7W_tests_sources *.cpp)
    set(console_tests_precompile_header "${CMAKE_CURRENT_SOURCE_DIR}/precompile/precompile_7W_tests.h")

    add_executable(${PROJECT_NAME} ${7W_tests_headers} ${7W_tests_sources} ${7W_tests_precompile_header})

    # Enabled warnings
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

    # Precompile headers
    target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${7W_tests_precompile_header}>")

    # Linked libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE Core AI "${TORCH_LIBRARIES}")

    # For nicer IDE views
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tests")
	
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)

    source_group(
      TREE ${CMAKE_CURRENT_SOURCE_DIR}
      PREFIX "Header Files"
      FILES ${7W_tests_headers})

    source_group(
      TREE ${CMAKE_CURRENT_SOURCE_DIR}
      PREFIX "Source Files"
      FILES ${7W_tests_sources})

    source_group(
      "Precompile Header File"
      FILES ${7W_tests_precompile_header})
endif()