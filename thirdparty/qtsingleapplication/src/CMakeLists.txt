set (SOURCES_LIB
    qtsingleapplication.cpp
    qtlocalpeer.cpp
)

set (EXTRA_HEADERS_LIB
    QtSingleApplication
)

add_library (qtsingleapplicationlib STATIC
    ${SOURCES_LIB}
    ${EXTRA_HEADERS_LIB}
)

target_link_libraries (qtsingleapplicationlib
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)

if (${ENABLE_QT5})
    qt5_use_modules(qtsingleapplicationlib Core Widgets Gui Network)
endif()
