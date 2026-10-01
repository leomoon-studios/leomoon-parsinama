cmake_minimum_required(VERSION 3.21)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata/VERSION"
)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/metadata/VERSION" PARSINAMA_VERSION)
string(STRIP "${PARSINAMA_VERSION}" PARSINAMA_VERSION)
if(NOT PARSINAMA_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "metadata/VERSION must contain a version such as 0.1.0")
endif()

project(LeoMoonParsiNama
    VERSION "${PARSINAMA_VERSION}"
    DESCRIPTION "Offline Persian poetry reader"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Concurrent
    Gui
    Qml
    Quick
    QuickControls2
    Sql
    PrintSupport
    Test
)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

set(PARSINAMA_APP_ID "com.leomoon.ParsiNama")

qt_add_library(parsinama_data STATIC
    src/data/CatalogPaths.cpp
    src/data/CatalogPaths.h
    src/data/CatalogRepository.cpp
    src/data/CatalogRepository.h
    src/data/CollectionListModel.cpp
    src/data/CollectionListModel.h
    src/data/PoemLoader.cpp
    src/data/PoemLoader.h
    src/data/PoetListModel.cpp
    src/data/PoetListModel.h
    src/services/SettingsStore.cpp
    src/services/SettingsStore.h
    src/services/UserDataPaths.cpp
    src/services/UserDataPaths.h
)
target_include_directories(parsinama_data PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(parsinama_data PUBLIC Qt6::Core Qt6::Concurrent Qt6::Sql)

qt_add_executable(leomoon_parsinama
    src/main.cpp
)

set_source_files_properties(qml/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
set_source_files_properties(qml/theme/AppTheme.qml PROPERTIES QT_RESOURCE_ALIAS theme/AppTheme.qml)
set_source_files_properties(qml/components/Typography.qml PROPERTIES QT_RESOURCE_ALIAS components/Typography.qml)
set_source_files_properties(qml/components/HeaderAction.qml PROPERTIES QT_RESOURCE_ALIAS components/HeaderAction.qml)
qt_add_qml_module(leomoon_parsinama
    URI LeoMoon.ParsiNama
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/Main.qml
        qml/theme/AppTheme.qml
        qml/components/Typography.qml
        qml/components/HeaderAction.qml
    RESOURCES
        assets/app-icon.svg
        "assets/fonts/Vazirmatn[wght].ttf"
        assets/fonts/OFL.txt
)

target_link_libraries(leomoon_parsinama PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sql
    Qt6::PrintSupport
    parsinama_data
)

target_compile_definitions(leomoon_parsinama PRIVATE
    PARSINAMA_VERSION="${PROJECT_VERSION}"
    PARSINAMA_APP_ID="${PARSINAMA_APP_ID}"
)

qt_add_executable(parsinama_catalog_builder src/tools/build_catalog.cpp)
set_target_properties(parsinama_catalog_builder PROPERTIES
    OUTPUT_NAME parsinama-catalog-builder
)
target_link_libraries(parsinama_catalog_builder PRIVATE Qt6::Core Qt6::Sql)

set_target_properties(leomoon_parsinama PROPERTIES
    OUTPUT_NAME leomoon-parsinama
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "LeoMoon ParsiNama"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${PARSINAMA_APP_ID}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
)

install(TARGETS leomoon_parsinama
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

if(UNIX AND NOT APPLE)
    install(FILES "packaging/linux/${PARSINAMA_APP_ID}.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
    )
endif()

if(BUILD_TESTING)
    qt_add_executable(parsinama_app_tests tests/app_tests.cpp)
    target_link_libraries(parsinama_app_tests PRIVATE Qt6::Core Qt6::Test)
    target_compile_definitions(parsinama_app_tests PRIVATE
        PARSINAMA_VERSION="${PROJECT_VERSION}"
    )

    add_test(NAME desktop_smoke COMMAND parsinama_app_tests)
    set_tests_properties(desktop_smoke PROPERTIES
        ENVIRONMENT "PARSINAMA_TEST_APP=$<TARGET_FILE:leomoon_parsinama>;QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=;QT_QUICK_BACKEND=software;QT_QUICK_CONTROLS_STYLE=Basic"
        TIMEOUT 15
    )

    qt_add_executable(parsinama_catalog_tests tests/catalog_tests.cpp)
    target_link_libraries(parsinama_catalog_tests PRIVATE Qt6::Core Qt6::Sql Qt6::Test)
    add_test(NAME catalog_builder COMMAND parsinama_catalog_tests)
    set_tests_properties(catalog_builder PROPERTIES
        ENVIRONMENT "PARSINAMA_CATALOG_BUILDER=$<TARGET_FILE:parsinama_catalog_builder>"
        TIMEOUT 15
    )

    qt_add_executable(parsinama_data_tests tests/data_tests.cpp)
    target_link_libraries(parsinama_data_tests PRIVATE Qt6::Core Qt6::Sql Qt6::Test parsinama_data)
    add_test(NAME native_data_access COMMAND parsinama_data_tests)
    set_tests_properties(native_data_access PROPERTIES TIMEOUT 20)

    qt_add_executable(parsinama_settings_tests tests/settings_tests.cpp)
    target_link_libraries(parsinama_settings_tests PRIVATE Qt6::Core Qt6::Test parsinama_data)
    add_test(NAME user_settings COMMAND parsinama_settings_tests)
endif()
