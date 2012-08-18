project (plantumlqeditor)

cmake_minimum_required (VERSION 2.8)

if(WIN32)
    add_definitions(-DUNICODE -DQT_LARGEFILE_SUPPORT)
endif()

set (CMAKE_BUILD_TYPE "Debug")

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/)
set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

set (QT_QMAKE_EXECUTABLE $ENV{QT_QMAKE_EXECUTABLE})
find_package (Qt4 4.8 REQUIRED)
include (UseQt4)
set (CMAKE_AUTOMOC TRUE)

set (GMOCK_ROOT ${CMAKE_SOURCE_DIR}/thirdparty/gmock-1.6.0)
set (GTEST_INCLUDE_DIR ${GMOCK_ROOT}/gtest/include)
set (GTEST_MAIN_LIBRARY gtest_main)
set (GTEST_LIBRARY gtest)
set (GMOCK_LIBRARY gmock)
enable_testing ()
find_package (GTest REQUIRED)
include_directories (${GMOCK_ROOT}/include ${GTEST_INCLUDE_DIR})

message ("CMAKE_BUILD_TYPE:  ${CMAKE_BUILD_TYPE}")
message ("Qt4 Headers:       ${QT_INCLUDES}")

include_directories (${CMAKE_SOURCE_DIR})
include_directories (${QT_INCLUDES})
include_directories (${CMAKE_SOURCE_DIR}/thirdparty/qtsingleapplication/src)

#-------------------------------------------------------------------------------
# add to SOURCES files that will be tested with GMock and GTest
#-------------------------------------------------------------------------------
set (SOURCES_LIB
    assistantxmlreader.cpp
    filecache.cpp
    recentdocuments.cpp
)

#-------------------------------------------------------------------------------
# add to EXTRA_HEADERS all headers but those for classes for QObject derived
# classes
#-------------------------------------------------------------------------------
set (EXTRA_HEADERS_LIB
)

add_library (plantumlqeditorlib
    ${SOURCES_LIB}
    ${EXTRA_HEADERS_LIB}
)

target_link_libraries (plantumlqeditorlib
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

add_subdirectory (thirdparty)
add_subdirectory (tests)

#-------------------------------------------------------------------------------
# the real app
#-------------------------------------------------------------------------------
set (SOURCES_APP
    main.cpp
    mainwindow.cpp
    preferencesdialog.cpp
    previewwidget.cpp
    utils.cpp
)

set (EXTRA_HEADERS_APP
    utils.h
)

set(FORMS_APP
    preferencesdialog.ui
)
qt4_wrap_ui(SOURCES_FORMS_APP ${FORMS_APP})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_executable(plantumlqeditor
    ${SOURCES_APP}
    ${SOURCES_FORMS_APP}
    ${EXTRA_HEADERS_APP}
)

target_link_libraries(plantumlqeditor
    ${QT_QTSVG_LIBRARY}
    plantumlqeditorlib
    qtsingleapplicationlib
)

#-------------------------------------------------------------------------------
# installer support
#-------------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "PlantUML-QEditor")
set(CPACK_PACKAGE_VENDOR "Ionutz Borcoman")
set(CPACK_PACKAGE_CONTACT "iborco@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PlantUML QEditor - a Qt editor for PlantUML")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_GENERATOR "DEB")
set(CPACK_GENERATOR "NSIS")
set(CPACK_GENERATOR "RPM")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ionutz Borcoman") #required

install(TARGETS plantumlqeditor RUNTIME DESTINATION bin)

include (CPack)
