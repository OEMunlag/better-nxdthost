cmake_minimum_required(VERSION 3.16)
project(nxdumptool_host VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/usbmanager.cpp
    src/progressdialog.cpp
)

set(HEADERS
    src/mainwindow.h
    src/usbmanager.h
    src/progressdialog.h
    src/usbcommands.h
)

include_directories(src)

add_executable(nxdumptool_host ${SOURCES} ${HEADERS})

target_link_libraries(nxdumptool_host
    Qt6::Core
    Qt6::Widgets
    ${LIBUSB_LIBRARIES}
)

target_include_directories(nxdumptool_host PRIVATE
    ${LIBUSB_INCLUDE_DIRS}
)

target_compile_definitions(nxdumptool_host PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
)

install(TARGETS nxdumptool_host
    RUNTIME DESTINATION bin
)
