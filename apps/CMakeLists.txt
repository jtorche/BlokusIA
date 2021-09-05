cmake_minimum_required(VERSION 3.8)

project(Blokus_UI DESCRIPTION "Blokus Game UI" LANGUAGES CXX)

option(BUILD_UI "Check it to generate the Blokus UI project." OFF)
if (BUILD_UI)
    set(QT_MAJOR_VERSION 5 CACHE STRING "Qt major version to use.")
    option(USE_QT_CREATOR "Check it if you use QtCreator as IDE." OFF)

    # The QtWidgets module find its own dependencies (QtCore and QtGui)
    find_package(Qt${QT_MAJOR_VERSION} COMPONENTS Widgets QUIET)

    # Create the target project only if Qt was found
    if (DEFINED Qt${QT_MAJOR_VERSION}Widgets_FOUND AND Qt${QT_MAJOR_VERSION}Widgets_FOUND)
        # Determine windeployqt for Windows platform
        if (WIN32 AND TARGET Qt${QT_MAJOR_VERSION}::qmake AND NOT TARGET Qt${QT_MAJOR_VERSION}::windeployqt AND NOT USE_QT_CREATOR)
            get_target_property(qt_qmake_location Qt${QT_MAJOR_VERSION}::qmake IMPORTED_LOCATION)

            execute_process(
                COMMAND "${qt_qmake_location}" -query QT_INSTALL_PREFIX
                RESULT_VARIABLE return_code
                OUTPUT_VARIABLE qt_installation_prefix
                OUTPUT_STRIP_TRAILING_WHITESPACE)

            set(imported_win_deploy_qt_location "${qt_installation_prefix}/bin/windeployqt.exe")

            if (EXISTS ${imported_win_deploy_qt_location})
                add_executable(Qt${QT_MAJOR_VERSION}::windeployqt IMPORTED)

                set_target_properties(Qt${QT_MAJOR_VERSION}::windeployqt PROPERTIES IMPORTED_LOCATION ${imported_win_deploy_qt_location})
            endif()
        endif()

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

        add_executable(${PROJECT_NAME} ${ui_headers} ${ui_sources} ${ui_uis})
        # To avoid having a console when running GUI application
        if (WIN32)
            set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
        endif()

        target_link_libraries(${PROJECT_NAME} PRIVATE Core IA)
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_MAJOR_VERSION}::Widgets)

        # Post build step to deploy needed stuff in output directory under Windows
        if (TARGET Qt${QT_MAJOR_VERSION}::windeployqt)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND Qt${QT_MAJOR_VERSION}::windeployqt $<TARGET_FILE:${PROJECT_NAME}>)
        endif()

        if (QT_MAJOR_VERSION EQUAL 6)
            qt_finalize_executable(${PROJECT_NAME})
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
    else()
        message("Could not find Qt${QT_MAJOR_VERSION}. ${PROJECT_NAME} project won't be generated.")
        message("Qt${QT_MAJOR_VERSION}_DIR must be set to path like: <QTDIR>/lib/cmake/Qt${QT_MAJOR_VERSION}.")
    endif()
endif()