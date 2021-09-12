cmake_minimum_required(VERSION 3.16)

project(Blokus_UI DESCRIPTION "Blokus Game UI" LANGUAGES CXX)

option(BUILD_UI "Check it to generate the Blokus UI project." OFF)
if (BUILD_UI)
    set(qt_major_version 5)
    option(USE_QT_CREATOR "Check it if you use QtCreator as IDE." OFF)

    # The QtWidgets module find its own dependencies (QtCore and QtGui)
    find_package(Qt${qt_major_version} COMPONENTS Widgets QUIET)

    # Create the target project only if Qt was found
    if (DEFINED Qt${qt_major_version}Widgets_FOUND AND Qt${qt_major_version}Widgets_FOUND)
        # Get Qt installation folder
        if (TARGET Qt${qt_major_version}::qmake)
            get_target_property(qt_qmake_location Qt${qt_major_version}::qmake IMPORTED_LOCATION)

            execute_process(
                COMMAND "${qt_qmake_location}" -query QT_INSTALL_PREFIX
                RESULT_VARIABLE return_code
                OUTPUT_VARIABLE qt_installation_prefix
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        endif()

        # Determine windeployqt for Windows platform
        if (WIN32 AND DEFINED qt_installation_prefix AND NOT TARGET Qt${qt_major_version}::windeployqt AND NOT USE_QT_CREATOR)
            set(imported_win_deploy_qt_location "${qt_installation_prefix}/bin/windeployqt.exe")

            if (EXISTS ${imported_win_deploy_qt_location})
                add_executable(Qt${qt_major_version}::windeployqt IMPORTED)

                set_target_properties(Qt${qt_major_version}::windeployqt PROPERTIES IMPORTED_LOCATION ${imported_win_deploy_qt_location})
            endif()

            # Check VCINSTALLDIR environment variable is present
            IF(DEFINED ENV{VCINSTALLDIR})
                set(VC_INSTALL_DIR $ENV{VCINSTALLDIR})
            else()
                set(VC_INSTALL_DIR "" CACHE PATH "Visual Studio VC installation path (example: C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC")
            endif()
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
        set(ui_resources "${CMAKE_CURRENT_SOURCE_DIR}/Resources/app.qrc")
        set(ui_precompile_header "${CMAKE_CURRENT_SOURCE_DIR}/precompile/precompile_ui.h")

        add_executable(${PROJECT_NAME} ${ui_headers} ${ui_sources} ${ui_uis} ${ui_precompile_header} ${ui_resources})
        # To avoid having a console when running GUI application
        if (WIN32)
            set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
        endif()

        # Add CAExcludePath to avoid warnings that comes from Qt external library
        if (MSVC AND DEFINED qt_installation_prefix)
            set_target_properties(${PROJECT_NAME} PROPERTIES VS_GLOBAL_CAExcludePath "${qt_installation_prefix}/include;$(CAExcludePath)")
        endif()

        # Enabled warnings
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

        # Precompile headers
        target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ui_precompile_header}>")

        # Linked libraries
        target_link_libraries(${PROJECT_NAME} PRIVATE Core IA)
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt${qt_major_version}::Widgets)

        # Post build step to deploy needed stuff in output directory under Windows
        if (TARGET Qt${qt_major_version}::windeployqt)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND set VCINSTALLDIR=${VC_INSTALL_DIR}
                COMMAND Qt${qt_major_version}::windeployqt $<TARGET_FILE:${PROJECT_NAME}>)
        endif()

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