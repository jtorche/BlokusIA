cmake_minimum_required(VERSION 3.16)

project(UI DESCRIPTION "Blokus UI library" LANGUAGES CXX)

if (BUILD_UI)
    include("${CMAKE_SOURCE_DIR}/cmake/QtHelpers.cmake")

    # The QtGui module
    find_package(Qt${qt_major_version} COMPONENTS Gui QUIET)

    # Create the target project only if Qt was found
    if (DEFINED Qt${qt_major_version}Gui_FOUND AND Qt${qt_major_version}Gui_FOUND)
        get_qt_installation_folder(${qt_major_version})

        # Note that headers are optional, and do not affect add_library, but they will
        # not show up in IDEs unless they are listed in add_library.
        file(GLOB_RECURSE ui_headers ${CMAKE_SOURCE_DIR}/include/UI/*.h)
        file(GLOB_RECURSE ui_sources *.cpp)
        set(ui_precompile_header "${CMAKE_CURRENT_SOURCE_DIR}/precompile/precompile_ui.h")

        add_library(${PROJECT_NAME} ${ui_headers} ${ui_sources} ${ui_precompile_header})

        avoid_qt_warnings(${PROJECT_NAME} ${qt_installation_folder})

        # We need this directory, and users of our library will need it too
        target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

        # Enabled warnings
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

        # Precompile headers
        target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ui_precompile_header}>")

        # Linked libraries
        target_link_libraries(${PROJECT_NAME} PRIVATE Core)
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt${qt_major_version}::Gui)

        # For nicer IDE views
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")

        source_group(
          TREE ${CMAKE_SOURCE_DIR}/include/UI
          PREFIX "Header Files"
          FILES ${ui_headers})

        source_group(
          TREE ${CMAKE_CURRENT_SOURCE_DIR}
          PREFIX "Source Files"
          FILES ${ui_sources})

        source_group(
          "Precompile Header File"
          FILES ${ui_precompile_header})
    else()
        message("Could not find Qt${qt_major_version}. ${PROJECT_NAME} project won't be generated.")
        message("Qt${qt_major_version}_DIR must be set to path like: <QTDIR>/lib/cmake/Qt${qt_major_version}.")
    endif()
endif()