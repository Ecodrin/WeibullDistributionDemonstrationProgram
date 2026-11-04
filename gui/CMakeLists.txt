cmake_minimum_required(VERSION 3.16)

project(gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Widgets Graphs Quick QuickWidgets Svg
)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets Graphs Quick QuickWidgets Svg
)

# QSS renders SVG arrows through Qt's SVG icon plugin.
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Svg)
endif()

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

add_subdirectory(libs)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(gui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        createfuzzyvaluewindow.h
        createfuzzyvaluewindow.cpp
        createfuzzyvaluewindow.ui

        # Убрали styles.css отсюда, это не C++ файл
    )

    # Ресурсы доступны приложению по адресам:
    # :/styles.css, :/up.svg и :/down.svg
    # BASE задаёт корень проекта, поэтому пути не зависят от build-directory.
    set(APP_RESOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/styles.css"
        "${CMAKE_CURRENT_SOURCE_DIR}/up.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/down.svg"
    )

    foreach(resource_file IN LISTS APP_RESOURCE_FILES)
        if(NOT EXISTS "${resource_file}")
            message(FATAL_ERROR "Не найден файл ресурса: ${resource_file}")
        endif()
    endforeach()

    qt_add_resources(gui "app_resources"
        PREFIX "/"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES ${APP_RESOURCE_FILES}
    )

else()
    if(ANDROID)
        add_library(gui SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(gui
            ${PROJECT_SOURCES}
        )
        # Для Qt5 стили добавляются через .qrc файл (см. примечание ниже)
    endif()
endif()

target_link_libraries(gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Graphs
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Svg
    weibull_distribution_math_library fuzzy_logic_library)

if(DEFINED VCPKG_INSTALLED_DIR
   AND DEFINED VCPKG_TARGET_TRIPLET)

    set(APP_VCPKG_TRIPLET_DIR
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")

    file(TO_CMAKE_PATH
        "${APP_VCPKG_TRIPLET_DIR}"
        APP_VCPKG_TRIPLET_DIR)

    target_compile_definitions(gui PRIVATE
        "APP_VCPKG_TRIPLET_DIR=\"${APP_VCPKG_TRIPLET_DIR}\""
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(gui PRIVATE Qt6::Svg)
endif()

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.gui)
endif()
set_target_properties(gui PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS gui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(gui)
endif()