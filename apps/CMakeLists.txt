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
        if (NOT USE_QT_CREATOR AND WIN32 AND DEFINED qt_installation_prefix AND NOT TARGET Qt${qt_major_version}::windeployqt)
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
        set(ui_resources
            "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.qrc"
            "${CMAKE_CURRENT_SOURCE_DIR}/themes/themes.qrc"
            "${CMAKE_CURRENT_SOURCE_DIR}/i18n/translations.qrc")
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

        # Linguist tools module
        option(GENERATE_TRANSLATIONS "Check it to generate/update the Blokus UI translation files." ON)
        find_package(Qt${qt_major_version} COMPONENTS LinguistTools QUIET)
        if (GENERATE_TRANSLATIONS AND DEFINED Qt${qt_major_version}LinguistTools_FOUND AND Qt${qt_major_version}LinguistTools_FOUND)
            # Function to generate/update ts and qm files searching recursively in SRC_DIR
            function(generate_app_translations CUSTOM_TARGET TS_DIR TS_FILES SRC_DIR)
                set(update_ts_target_name ${CUSTOM_TARGET}_update_ts)
                set(update_qm_target_name ${CUSTOM_TARGET}_update_qm)

                add_custom_target(${update_ts_target_name}
                    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive ${SRC_DIR} -ts ${TS_FILES}
                    WORKING_DIRECTORY ${TS_DIR})

                add_custom_target(${update_qm_target_name}
                    COMMAND ${Qt5_LRELEASE_EXECUTABLE} ${TS_FILES}
                    WORKING_DIRECTORY ${TS_DIR})

                add_dependencies(${update_qm_target_name} ${update_ts_target_name})
                add_dependencies(${CUSTOM_TARGET} ${update_qm_target_name})
            endfunction()

            set(ts_dir "${CMAKE_CURRENT_SOURCE_DIR}/i18n")
            set(ts_files "${ts_dir}/blokus_fr.ts")

            generate_app_translations(${PROJECT_NAME} ${ts_dir} ${ts_files} ${CMAKE_CURRENT_SOURCE_DIR})
        endif()

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