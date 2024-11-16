cmake_minimum_required(VERSION 3.16)

project(SerialPortHelper_Asio VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/13169/vcpkg/scripts/buildsystems/vcpkg.cmake" )

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort)
find_package(Boost REQUIRED COMPONENTS system asio)


set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SerialPortHelper_Asio
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Res.qrc
        cserialportmanager.h cserialportmanager.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SerialPortHelper_Asio APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(SerialPortHelper_Asio SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SerialPortHelper_Asio
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(SerialPortHelper_Asio PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Boost::system Boost::asio Qt${QT_VERSION_MAJOR}::SerialPort)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SerialPortHelper_Asio)
endif()
if(NOT BUNDLE_ID_OPTION)
  set(BUNDLE_ID_OPTION)
endif()
if(WIN32)
    target_link_libraries(SerialPortHelper_Asio PRIVATE ws2_32)
endif()
set_target_properties(SerialPortHelper_Asio PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SerialPortHelper_Asio
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SerialPortHelper_Asio)
endif()
