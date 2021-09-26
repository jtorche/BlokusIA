set(qt_major_version 5)
option(USE_QT_CREATOR "Check it if you use QtCreator as IDE." OFF)

# Get Qt installation folder
function(get_qt_installation_folder QT_VERSION)
    if (TARGET Qt${QT_VERSION}::qmake)
        get_target_property(qt_qmake_location Qt${QT_VERSION}::qmake IMPORTED_LOCATION)

        execute_process(
            COMMAND "${qt_qmake_location}" -query QT_INSTALL_PREFIX
            RESULT_VARIABLE return_code
            OUTPUT_VARIABLE qt_installation_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        set(qt_installation_folder ${qt_installation_prefix} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Failed to retrieve Qt installation folder.")
    endif()
endfunction()

function(setup_windeployqt_target QT_VERSION QT_FOLDER)
    set(imported_win_deploy_qt_location "${QT_FOLDER}/bin/windeployqt.exe")

    if (EXISTS ${imported_win_deploy_qt_location})
        add_executable(Qt${QT_VERSION}::windeployqt IMPORTED)

        set_target_properties(Qt${QT_VERSION}::windeployqt PROPERTIES IMPORTED_LOCATION ${imported_win_deploy_qt_location})
    endif()

    # Check VCINSTALLDIR environment variable is present
    IF(DEFINED ENV{VCINSTALLDIR})
        set(VC_INSTALL_DIR $ENV{VCINSTALLDIR} PARENT_SCOPE)
    else()
        set(VC_INSTALL_DIR "" CACHE PATH "Visual Studio VC installation path (example: C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC")
    endif()
endfunction()

function(avoid_qt_warnings CUSTOM_TARGET QT_FOLDER)
    # Add CAExcludePath to avoid warnings that comes from Qt external library
    if (MSVC)
        set_target_properties(${CUSTOM_TARGET} PROPERTIES VS_GLOBAL_CAExcludePath "${QT_FOLDER}/include;$(CAExcludePath)")
    endif()
endfunction()

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

    # For nicer IDE views
    set_target_properties(${update_ts_target_name} PROPERTIES FOLDER "i18n")
    set_target_properties(${update_qm_target_name} PROPERTIES FOLDER "i18n")
endfunction()

# Post build step to deploy needed stuff in output directory under Windows
function(add_windeployqt_postbuild CUSTOM_TARGET QT_VERSION VC_INSTALL_DIR)
    if (TARGET Qt${QT_VERSION}::windeployqt)
        add_custom_command(TARGET ${CUSTOM_TARGET} POST_BUILD
            COMMAND set VCINSTALLDIR=${VC_INSTALL_DIR}
            COMMAND Qt${QT_VERSION}::windeployqt $<TARGET_FILE:${CUSTOM_TARGET}>)
    endif()
endfunction()