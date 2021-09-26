cmake_minimum_required(VERSION 3.16)

project(UI_Tests DESCRIPTION "Graphic application to easily test Blokus AI" LANGUAGES CXX)

if (BUILD_UI AND BUILD_TESTING)
    include("${CMAKE_SOURCE_DIR}/cmake/QtHelpers.cmake")

    # The QtWidgets module find its own dependencies (QtCore and QtGui)
    find_package(Qt${qt_major_version} COMPONENTS Widgets QUIET)

    # Create the target project only if Qt was found
    if (DEFINED Qt${qt_major_version}Widgets_FOUND AND Qt${qt_major_version}Widgets_FOUND)
        get_qt_installation_folder(${qt_major_version})

        # Setup windeployqt target for Windows platform
        if (NOT USE_QT_CREATOR AND WIN32 AND NOT TARGET Qt${qt_major_version}::windeployqt)
            setup_windeployqt_target(${qt_major_version} ${qt_installation_folder})
        endif()

        # IDE filter for generated files
        set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

        # As moc files are generated in the binary dir, tell CMake
        # to always look for includes there
        set(CMAKE_INCLUDE_CURRENT_DIR ON)

        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)

        # Sources
        file(GLOB_RECURSE ui_tests_headers *.h)
        file(GLOB_RECURSE ui_tests_sources *.cpp)
        set(ui_tests_precompile_header "${CMAKE_CURRENT_SOURCE_DIR}/precompile/precompile_ui_tests.h")
        set(app_sources ${ui_tests_headers} ${ui_tests_sources} ${ui_tests_precompile_header})
        add_executable(${PROJECT_NAME} ${app_sources})

        avoid_qt_warnings(${PROJECT_NAME} ${qt_installation_folder})

        # Enabled warnings
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

        # Precompile headers
        target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ui_tests_precompile_header}>")

        # Linked libraries
        target_link_libraries(${PROJECT_NAME} PRIVATE Core AI)
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt${qt_major_version}::Widgets)

        add_windeployqt_postbuild(${PROJECT_NAME} ${qt_major_version} ${VC_INSTALL_DIR})

        # For nicer IDE views
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tests")

        source_group(
          TREE ${CMAKE_CURRENT_SOURCE_DIR}
          PREFIX "Header Files"
          FILES ${ui_tests_headers})

        source_group(
          TREE ${CMAKE_CURRENT_SOURCE_DIR}
          PREFIX "Source Files"
          FILES ${ui_tests_sources})

        source_group(
          "Resource Files"
          FILES ${ui_resources})

        source_group(
          "Precompile Header File"
          FILES ${ui_tests_precompile_header})
    else()
        message("Could not find Qt${qt_major_version}. ${PROJECT_NAME} project won't be generated.")
        message("Qt${qt_major_version}_DIR must be set to path like: <QTDIR>/lib/cmake/Qt${qt_major_version}.")
    endif()
endif()