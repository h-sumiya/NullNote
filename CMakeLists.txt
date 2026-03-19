cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED NULLNOTE_VERSION)
    set(NULLNOTE_VERSION 0.1.1)
endif()

project(NullNote VERSION ${NULLNOTE_VERSION} LANGUAGES CXX)

if(WIN32)
    set(CMAKE_INSTALL_BINDIR "." CACHE STRING "Install path for runtime binaries" FORCE)
endif()

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_standard_project_setup()
endif()

set(NULL_NOTE_SOURCES
    assets/appicon.qrc
    src/linkawareplaintextedit.cpp
    src/linkawareplaintextedit.h
    src/main.cpp
    src/notewindow.cpp
    src/notewindow.h
)

if(WIN32)
    set(NULL_NOTE_WIN_RC "${CMAKE_CURRENT_BINARY_DIR}/nullnote-icon.rc")
    file(GENERATE
        OUTPUT "${NULL_NOTE_WIN_RC}"
        CONTENT "IDI_ICON1 ICON \"${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico\"\n"
    )
    list(APPEND NULL_NOTE_SOURCES "${NULL_NOTE_WIN_RC}")
endif()

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(NullNote WIN32 ${NULL_NOTE_SOURCES})
else()
    add_executable(NullNote WIN32 ${NULL_NOTE_SOURCES})
endif()

target_link_libraries(NullNote PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set(NULLNOTE_THIRD_PARTY_NOTICES "${CMAKE_CURRENT_BINARY_DIR}/THIRD_PARTY_NOTICES.txt")
file(GENERATE
    OUTPUT "${NULLNOTE_THIRD_PARTY_NOTICES}"
    CONTENT
"NullNote bundles Qt runtime libraries.\n\
\n\
Qt licensing information:\n\
- https://www.qt.io/licensing/\n\
- https://doc.qt.io/qt-6/lgpl.html\n"
)

install(TARGETS NullNote
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${NULLNOTE_THIRD_PARTY_NOTICES}"
    DESTINATION .
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_generate_deploy_app_script(
        TARGET NullNote
        OUTPUT_SCRIPT nullnote_deploy_script
    )
    install(SCRIPT "${nullnote_deploy_script}")
endif()

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "NullNote")
set(CPACK_PACKAGE_VENDOR "h-sumiya")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "NullNote-${PROJECT_VERSION}-windows-x64")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

include(CPack)
