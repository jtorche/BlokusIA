cmake_minimum_required(VERSION 3.16)

project(ML_Tests DESCRIPTION "Console application to easily test Machine learning on Blokus AI" LANGUAGES CXX)

if (BUILD_ML_PROJECTS AND BUILD_TESTING)
    file(GLOB_RECURSE ml_tests_headers *.h)
    file(GLOB_RECURSE ml_tests_sources *.cpp)
    set(ml_test_precompile_header "${CMAKE_CURRENT_SOURCE_DIR}/precompile/precompile_ml_tests.h")

    add_executable(${PROJECT_NAME} ${ml_tests_headers} ${ml_tests_sources} ${console_ml_precompile_header})

    # Enabled warnings
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W3 /wd4100 /wd4624>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>)

    # Precompile headers
    target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ml_test_precompile_header}>")
	
    # For nicer IDE views
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "ml_tests")
	
	target_link_libraries(${PROJECT_NAME} PRIVATE ML)

    source_group(
      TREE ${CMAKE_CURRENT_SOURCE_DIR}
      PREFIX "Header Files"
      FILES ${ml_tests_headers})

    source_group(
      TREE ${CMAKE_CURRENT_SOURCE_DIR}
      PREFIX "Source Files"
      FILES ${ml_tests_sources})

    source_group(
      "Precompile Header File"
      FILES ${ml_test_precompile_header})
endif()