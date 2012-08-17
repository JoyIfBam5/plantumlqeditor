project (plantuml_qeditor_test)

cmake_minimum_required (VERSION 2.8)

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

#-------------------------------------------------------------------------------
# add to SOURCES files that will be tested with GMock and GTest
#-------------------------------------------------------------------------------
set (SOURCES
    assistantxmlreader.cpp
    filecache.cpp
    recentdocuments.cpp
)

#-------------------------------------------------------------------------------
# add to EXTRA_HEADERS all headers but those for classes for QObject derived
# classes
#-------------------------------------------------------------------------------
set (EXTRA_HEADERS
)

add_library (plantuml_qeditor
    ${SOURCES}
    ${EXTRA_HEADERS}
)

target_link_libraries (plantuml_qeditor
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

add_subdirectory (thirdparty)
add_subdirectory (tests)
