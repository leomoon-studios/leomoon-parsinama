foreach(required BUILD_DIR SOURCE_DIR ARCH LINUXDEPLOY QMAKE VERSION)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is required")
    endif()
endforeach()

set(app_dir "${BUILD_DIR}/package/linux/AppDir")
file(REMOVE_RECURSE "${app_dir}")
file(MAKE_DIRECTORY "${app_dir}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E env "DESTDIR=${app_dir}"
    "${CMAKE_COMMAND}" --install "${BUILD_DIR}" --prefix /usr --component Application
    RESULT_VARIABLE install_result)
if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "Could not stage the ParsiNama AppImage")
endif()
set(staged_catalog "${app_dir}/usr/share/leomoon-parsinama/parsinama-catalog.sqlite")
file(REMOVE "${staged_catalog}")
file(CREATE_LINK "${BUILD_DIR}/parsinama-catalog.sqlite" "${staged_catalog}" COPY_ON_ERROR)

execute_process(COMMAND "${QMAKE}" -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE plugin_dir OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE plugin_result)
if(NOT plugin_result EQUAL 0 OR NOT EXISTS "${plugin_dir}/sqldrivers/libqsqlite.so")
    message(FATAL_ERROR "The Qt installation has no SQLite driver plugin")
endif()
set(filtered_plugins "${BUILD_DIR}/package/linux/qt-plugins")
file(REMOVE_RECURSE "${filtered_plugins}")
file(MAKE_DIRECTORY "${filtered_plugins}/sqldrivers" "${filtered_plugins}/printsupport")
# PDF export works without a platform printer plugin. Bundling the host CUPS plugin
# also pulls in printer libraries that are not portable across Linux distributions.
file(GLOB plugin_groups LIST_DIRECTORIES true "${plugin_dir}/*")
foreach(plugin_group IN LISTS plugin_groups)
    if(IS_DIRECTORY "${plugin_group}")
        get_filename_component(group_name "${plugin_group}" NAME)
        if(NOT group_name STREQUAL "sqldrivers" AND NOT group_name STREQUAL "printsupport")
            file(CREATE_LINK "${plugin_group}" "${filtered_plugins}/${group_name}" SYMBOLIC)
        endif()
    endif()
endforeach()
file(CREATE_LINK "${plugin_dir}/sqldrivers/libqsqlite.so"
    "${filtered_plugins}/sqldrivers/libqsqlite.so" SYMBOLIC)
set(filtered_qmake "${BUILD_DIR}/package/linux/qmake-filtered")
file(WRITE "${filtered_qmake}"
    "#!/bin/sh\nif [ \"\$1\" = '-query' ] && [ \"\$2\" = 'QT_INSTALL_PLUGINS' ]; then\n"
    "  printf '%s\\n' '${filtered_plugins}'\n  exit 0\nfi\n"
    "if [ \"\$1\" = '-query' ] && [ \"\$#\" -eq 1 ]; then\n"
    "  '${QMAKE}' -query | sed 's|^QT_INSTALL_PLUGINS:.*$|QT_INSTALL_PLUGINS:${filtered_plugins}|'\n"
    "  exit \$?\nfi\nexec '${QMAKE}' \"\$@\"\n")
file(CHMOD "${filtered_qmake}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
file(MAKE_DIRECTORY "${app_dir}/usr/bin/sqldrivers")
file(COPY_FILE "${plugin_dir}/sqldrivers/libqsqlite.so"
    "${app_dir}/usr/bin/sqldrivers/libqsqlite.so")

set(extra_platform_plugins libqoffscreen.so)
file(GLOB wayland_plugins "${plugin_dir}/platforms/libqwayland*.so")
foreach(wayland_plugin IN LISTS wayland_plugins)
    get_filename_component(plugin_name "${wayland_plugin}" NAME)
    list(APPEND extra_platform_plugins "${plugin_name}")
endforeach()
list(JOIN extra_platform_plugins ";" platform_plugin_list)
set(ENV{APPIMAGE_EXTRACT_AND_RUN} 1)
set(ENV{EXTRA_PLATFORM_PLUGINS} "${platform_plugin_list}")
set(ENV{QMAKE} "${filtered_qmake}")
set(ENV{QML_SOURCES_PATHS} "${SOURCE_DIR}/qml")
set(ENV{NO_STRIP} 1)
set(ENV{LDAI_OUTPUT} "${BUILD_DIR}/package/linux/leomoon-parsinama-${VERSION}-linux-${ARCH}.AppImage")
execute_process(COMMAND "${LINUXDEPLOY}"
    --appdir "${app_dir}"
    --executable "${app_dir}/usr/bin/leomoon-parsinama"
    --desktop-file "${app_dir}/usr/share/applications/com.leomoon.ParsiNama.desktop"
    --icon-file "${app_dir}/usr/share/icons/hicolor/scalable/apps/com.leomoon.ParsiNama.svg"
    --plugin qt --output appimage
    WORKING_DIRECTORY "${BUILD_DIR}/package/linux"
    OUTPUT_FILE "${BUILD_DIR}/package/linux/linuxdeploy.log"
    ERROR_FILE "${BUILD_DIR}/package/linux/linuxdeploy.log"
    RESULT_VARIABLE appimage_result)
if(NOT appimage_result EQUAL 0)
    file(STRINGS "${BUILD_DIR}/package/linux/linuxdeploy.log" deploy_lines)
    list(LENGTH deploy_lines deploy_line_count)
    math(EXPR first_line "${deploy_line_count} - 20")
    if(first_line LESS 0)
        set(first_line 0)
    endif()
    list(SUBLIST deploy_lines ${first_line} -1 recent_lines)
    string(JOIN "\n" recent_output ${recent_lines})
    message(FATAL_ERROR "linuxdeploy could not create the ParsiNama AppImage:\n${recent_output}")
endif()
