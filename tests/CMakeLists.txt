include_directories(
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

macro(register_test APP)
    add_test(${APP} ${EXECUTABLE_OUTPUT_PATH}${APP})
endmacro(register_test)

#-------------------------------------------------------------------------------
# test-assistantxmlreader
#-------------------------------------------------------------------------------

add_executable(test-assistantxmlreader
    main.cpp
    assistantxmlreadertest.cpp
)

target_link_libraries(test-assistantxmlreader
    ${GMOCK_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    plantuml_qeditor
)

register_test(test-assistantxmlreader)

#-------------------------------------------------------------------------------
# test-filecache
#-------------------------------------------------------------------------------

add_executable(test-filecache
    main.cpp
    filecachetest.cpp
)

target_link_libraries(test-filecache
    ${GMOCK_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    plantuml_qeditor
)

register_test(test-filecache)
