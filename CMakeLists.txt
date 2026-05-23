cmake_minimum_required(VERSION 3.19)
project(PRJ1 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(PRJ1
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    CalendarCore.cpp CalendarCore.h
    LunarCore.cpp LunarCore.h
    MemoManager.cpp MemoManager.h
)

target_link_libraries(PRJ1
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS PRJ1
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET PRJ1
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
