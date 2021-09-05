cmake_minimum_required(VERSION 3.8)

project(IA_Tests DESCRIPTION "Console application to easily test Blokus IA" LANGUAGES CXX)

option(BUILD_TESTING "Check it to generate the console Blokus test project." ON)
if (BUILD_TESTING)
    file(GLOB_RECURSE tests_ia_headers *.h)
    file(GLOB_RECURSE tests_ia_sources *.cpp)

    add_executable(${PROJECT_NAME} ${tests_ia_headers} ${tests_ia_sources})

    target_link_libraries(${PROJECT_NAME} PRIVATE Core IA)

    # For nicer IDE views
    source_group(
      TREE ${CMAKE_CURRENT_SOURCE_DIR}
      PREFIX "Header Files"
      FILES ${tests_ia_headers})

    source_group(
      TREE ${CMAKE_CURRENT_SOURCE_DIR}
      PREFIX "Source Files"
      FILES ${tests_ia_sources})
endif()