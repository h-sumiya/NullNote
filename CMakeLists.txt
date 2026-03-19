cmake_minimum_required(VERSION 3.21)

project(NullNote LANGUAGES CXX)

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
    src/linkawareplaintextedit.cpp
    src/linkawareplaintextedit.h
    src/main.cpp
    src/notewindow.cpp
    src/notewindow.h
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(NullNote WIN32 ${NULL_NOTE_SOURCES})
else()
    add_executable(NullNote WIN32 ${NULL_NOTE_SOURCES})
endif()

target_link_libraries(NullNote PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

