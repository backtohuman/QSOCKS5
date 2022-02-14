# https://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 3.16)

project(QSOCKS5 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)

file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/QSOCKS5/*.cpp)
set(RESOURCE_FILES ${PROJECT_SOURCE_DIR}/QSOCKS5/qsocks5.qrc)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${RESOURCE_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/QSOCKS5/stdafx.h)

# link qt
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets Qt5::Network)

if (APPLE)
    #set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_BUNDLE TRUE)
endif()

#
# test
#
add_executable(client client/main.cpp)
target_link_libraries(client PRIVATE Qt5::Core Qt5::Widgets Qt5::Network)