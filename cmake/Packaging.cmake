set(PARSINAMA_CATALOG_FILE "${CMAKE_CURRENT_BINARY_DIR}/parsinama-catalog.sqlite")
add_custom_command(
    OUTPUT "${PARSINAMA_CATALOG_FILE}"
    COMMAND "$<TARGET_FILE:parsinama_catalog_builder>"
        --data-root "${CMAKE_CURRENT_SOURCE_DIR}/data"
        --output "${PARSINAMA_CATALOG_FILE}"
    DEPENDS parsinama_catalog_builder "${CMAKE_CURRENT_SOURCE_DIR}/data/manifest.json"
    COMMENT "Generate the searchable ParsiNama catalog from data/"
    VERBATIM
)
add_custom_target(parsinama_catalog DEPENDS "${PARSINAMA_CATALOG_FILE}")
add_dependencies(leomoon_parsinama parsinama_catalog)

install(TARGETS leomoon_parsinama
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT Application
)

if(APPLE)
    install(FILES "${PARSINAMA_CATALOG_FILE}"
        DESTINATION "LeoMoon ParsiNama.app/Contents/Resources"
        COMPONENT Content
    )
elseif(WIN32)
    install(FILES "${PARSINAMA_CATALOG_FILE}"
        DESTINATION "${CMAKE_INSTALL_BINDIR}/data"
        COMPONENT Content
    )
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/app-icon.ico"
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT Application
    )
else()
    install(FILES "${PARSINAMA_CATALOG_FILE}"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/leomoon-parsinama"
        COMPONENT Content
    )
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/${PARSINAMA_APP_ID}.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
        COMPONENT Application
    )
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/app-icon.svg"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
        RENAME "${PARSINAMA_APP_ID}.svg"
        COMPONENT Application
    )
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/SOURCES.md"
              "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/leomoon-parsinama"
    COMPONENT Application
)

if(WIN32)
    enable_language(RC)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/leomoon-parsinama.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/packaging/windows/leomoon-parsinama.rc" @ONLY)
    target_sources(leomoon_parsinama PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/packaging/windows/leomoon-parsinama.rc")

    if(NOT DEFINED PARSINAMA_WINDOWS_RELEASE_ARCH)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|arm64|aarch64)$")
            set(PARSINAMA_WINDOWS_RELEASE_ARCH arm64)
        else()
            set(PARSINAMA_WINDOWS_RELEASE_ARCH x64)
        endif()
    endif()
    if(PARSINAMA_WINDOWS_RELEASE_ARCH STREQUAL "arm64")
        set(PARSINAMA_WINDOWS_INNO_ARCHITECTURE arm64)
    elseif(PARSINAMA_WINDOWS_RELEASE_ARCH STREQUAL "x64")
        set(PARSINAMA_WINDOWS_INNO_ARCHITECTURE x64compatible)
    else()
        message(FATAL_ERROR "Unsupported Windows release architecture: ${PARSINAMA_WINDOWS_RELEASE_ARCH}")
    endif()
    set(PARSINAMA_WINDOWS_STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/windows/stage")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/leomoon-parsinama.iss.in"
        "${CMAKE_CURRENT_BINARY_DIR}/packaging/windows/leomoon-parsinama.iss" @ONLY)
    get_target_property(parsinama_qmake Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(parsinama_qt_bin_dir "${parsinama_qmake}" DIRECTORY)
    find_program(PARSINAMA_WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${parsinama_qt_bin_dir}")
    find_program(PARSINAMA_INNO_SETUP_EXECUTABLE NAMES ISCC ISCC.exe
        HINTS "C:/Program Files (x86)/Inno Setup 6")
    if(PARSINAMA_WINDEPLOYQT_EXECUTABLE)
        add_custom_target(deploy_windows
            COMMAND "${CMAKE_COMMAND}"
                "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
                "-DSTAGE_DIR=${PARSINAMA_WINDOWS_STAGE_DIR}"
                "-DWINDEPLOYQT=${PARSINAMA_WINDEPLOYQT_EXECUTABLE}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
                "-DCONFIG=$<CONFIG>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/deploy.cmake"
            DEPENDS leomoon_parsinama parsinama_catalog
            VERBATIM)
        if(PARSINAMA_INNO_SETUP_EXECUTABLE)
            add_custom_target(package_windows
                COMMAND "${PARSINAMA_INNO_SETUP_EXECUTABLE}"
                    "${CMAKE_CURRENT_BINARY_DIR}/packaging/windows/leomoon-parsinama.iss"
                DEPENDS deploy_windows
                VERBATIM)
        endif()
    endif()
elseif(APPLE)
    set(PARSINAMA_MAC_ICON "${CMAKE_CURRENT_BINARY_DIR}/packaging/macos/app-icon.icns")
    add_custom_command(OUTPUT "${PARSINAMA_MAC_ICON}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/packaging/macos"
        COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/generate-icon.sh"
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/app-icon.svg" "${PARSINAMA_MAC_ICON}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/app-icon.svg"
                "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/generate-icon.sh"
        VERBATIM)
    set_source_files_properties("${PARSINAMA_MAC_ICON}" PROPERTIES
        GENERATED TRUE MACOSX_PACKAGE_LOCATION Resources)
    target_sources(leomoon_parsinama PRIVATE "${PARSINAMA_MAC_ICON}")
    set_target_properties(leomoon_parsinama PROPERTIES MACOSX_BUNDLE_ICON_FILE app-icon.icns)
    get_target_property(parsinama_qmake Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(parsinama_qt_bin_dir "${parsinama_qmake}" DIRECTORY)
    find_program(PARSINAMA_MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${parsinama_qt_bin_dir}")
    if(PARSINAMA_MACDEPLOYQT_EXECUTABLE)
        add_custom_target(package_macos
            COMMAND "${CMAKE_COMMAND}"
                "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
                "-DMACDEPLOYQT=${PARSINAMA_MACDEPLOYQT_EXECUTABLE}"
                "-DVERSION=${PROJECT_VERSION}"
                "-DCATALOG=${PARSINAMA_CATALOG_FILE}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/package.cmake"
            DEPENDS leomoon_parsinama parsinama_catalog
            VERBATIM)
    endif()
elseif(UNIX)
    get_target_property(parsinama_qmake Qt6::qmake IMPORTED_LOCATION)
    find_program(PARSINAMA_LINUXDEPLOY_EXECUTABLE linuxdeploy)
    if(PARSINAMA_LINUXDEPLOY_EXECUTABLE)
        add_custom_target(package_appimage
            COMMAND "${CMAKE_COMMAND}"
                "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
                "-DARCH=${CMAKE_SYSTEM_PROCESSOR}"
                "-DLINUXDEPLOY=${PARSINAMA_LINUXDEPLOY_EXECUTABLE}"
                "-DQMAKE=${parsinama_qmake}"
                "-DVERSION=${PROJECT_VERSION}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/build-appimage.cmake"
            DEPENDS leomoon_parsinama parsinama_catalog
            VERBATIM)
    endif()
endif()
