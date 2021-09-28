cmake_minimum_required(VERSION 3.16)

project(Blokus DESCRIPTION "Blokus Game" LANGUAGES CXX)

if (BUILD_UI)
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
        file(GLOB_RECURSE ui_headers *.h)
        file(GLOB_RECURSE ui_sources *.cpp)
        file(GLOB_RECURSE ui_uis *.ui)
        set(ui_resources
            "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.qrc"
            "${CMAKE_CURRENT_SOURCE_DIR}/resources/dark/style.qrc"
            "${CMAKE_CURRENT_SOURCE_DIR}/resources/light/style.qrc"
            "${CMAKE_CURRENT_SOURCE_DIR}/i18n/translations.qrc")
        set(ui_precompile_header "${CMAKE_CURRENT_SOURCE_DIR}/precompile/precompile_ui.h")
        set(app_sources ${ui_headers} ${ui_sources} ${ui_uis} ${ui_precompile_header} ${ui_resources})

        if (WIN32)
            set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
            add_executable(${PROJECT_NAME} ${app_sources} ${app_icon_resource_windows})

            # To avoid having a console when running GUI application
            set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
        else()
            add_executable(${PROJECT_NAME} ${app_sources})
        endif()

        avoid_qt_warnings(${PROJECT_NAME} ${qt_installation_folder})

        # Enabled warnings
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

        # Precompile headers
        target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ui_precompile_header}>")

        # Linked libraries
        target_link_libraries(${PROJECT_NAME} PRIVATE Core AI)
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt${qt_major_version}::Widgets)

        # Linguist tools module
        option(GENERATE_TRANSLATIONS "Check it to generate/update the Blokus UI translation files." ON)
        find_package(Qt${qt_major_version} COMPONENTS LinguistTools QUIET)
        if (GENERATE_TRANSLATIONS AND DEFINED Qt${qt_major_version}LinguistTools_FOUND AND Qt${qt_major_version}LinguistTools_FOUND)
            set(ts_dir "${CMAKE_CURRENT_SOURCE_DIR}/i18n")
            set(ts_files "${ts_dir}/blokus_fr.ts")

            generate_app_translations(${PROJECT_NAME} ${ts_dir} ${ts_files} ${CMAKE_CURRENT_SOURCE_DIR})
        endif()

        add_windeployqt_postbuild(${PROJECT_NAME} ${qt_major_version} "${VC_INSTALL_DIR}")

        # For nicer IDE views
        source_group(
          TREE ${CMAKE_CURRENT_SOURCE_DIR}
          PREFIX "Header Files"
          FILES ${ui_headers})

        source_group(
          TREE ${CMAKE_CURRENT_SOURCE_DIR}
          PREFIX "Source Files"
          FILES ${ui_sources})

        source_group(
          TREE ${CMAKE_CURRENT_SOURCE_DIR}
          PREFIX "View Files"
          FILES ${ui_uis})

        source_group(
          "Resource Files"
          FILES ${ui_resources})

        source_group(
          "Precompile Header File"
          FILES ${ui_precompile_header})
    else()
        message("Could not find Qt${qt_major_version}. ${PROJECT_NAME} project won't be generated.")
        message("Qt${qt_major_version}_DIR must be set to path like: <QTDIR>/lib/cmake/Qt${qt_major_version}.")
    endif()
endif()