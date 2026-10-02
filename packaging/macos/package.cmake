foreach(required BUILD_DIR SOURCE_DIR MACDEPLOYQT VERSION CATALOG)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is required")
    endif()
endforeach()
set(app "${BUILD_DIR}/LeoMoon ParsiNama.app")
set(resources "${app}/Contents/Resources")
set(output_dir "${BUILD_DIR}/package/macos")
if(NOT EXISTS "${app}/Contents/MacOS/LeoMoon ParsiNama" OR NOT EXISTS "${CATALOG}")
    message(FATAL_ERROR "Build the ParsiNama app and catalog before packaging")
endif()
file(MAKE_DIRECTORY "${resources}/licenses" "${output_dir}")
file(REMOVE "${resources}/parsinama-catalog.sqlite")
file(CREATE_LINK "${CATALOG}" "${resources}/parsinama-catalog.sqlite" COPY_ON_ERROR)
file(COPY_FILE "${SOURCE_DIR}/SOURCES.md" "${resources}/licenses/SOURCES.md" ONLY_IF_DIFFERENT)
file(COPY_FILE "${SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
    "${resources}/licenses/THIRD_PARTY_NOTICES.md" ONLY_IF_DIFFERENT)
execute_process(COMMAND "${MACDEPLOYQT}" "${app}"
    "-qmldir=${SOURCE_DIR}/qml" -always-overwrite
    RESULT_VARIABLE deploy_result)
if(NOT deploy_result EQUAL 0)
    message(FATAL_ERROR "macdeployqt failed")
endif()
if(NOT EXISTS "${app}/Contents/PlugIns/sqldrivers/libqsqlite.dylib")
    message(FATAL_ERROR "The deployed macOS app has no SQLite driver")
endif()
execute_process(COMMAND hdiutil create -volname "LeoMoon ParsiNama"
    -srcfolder "${app}" -ov -format UDZO
    "${output_dir}/leomoon-parsinama-${VERSION}-macos-universal.dmg"
    RESULT_VARIABLE dmg_result)
if(NOT dmg_result EQUAL 0)
    message(FATAL_ERROR "Could not create the ParsiNama DMG")
endif()
