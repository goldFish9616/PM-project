cmake_minimum_required(VERSION 3.5)

project(DbahnLogConverter2 VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)


# for QStringRef
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)

set(PROJECT_SOURCES
        main.cpp
        conv1.cpp
        conv1.h
        conv1.ui
        #
        crs_log_processor1.h
        crs_log_processor1.cpp

        xiQFiles.cpp
        xiQFiles.h

        #xi db

        xi/db/dbmain.cpp
        xi/db/dbmain.h

        #xi/db/sqlite/sqlite3_wrapper.c
        xi/db/sqlite/sqlite3.c
        #xi/db/sqlite/sqlite3.h
        #xi/db/sqlite/sqlite3ext.h
        xi/db/sqlite/xidbsqlite.cpp
        boost/config/select_stdlib_config.hpp
        boost/config/select_compiler_config.hpp
        boost/config/select_platform_config.hpp
        boost/config/stdlib/libstdcpp3.hpp
        boost/config.hpp
        boost/config/suffix.hpp
        boost/config/user.hpp
        boost/crc.hpp
        boost/cstdint.hpp
        boost/detail/workaround.hpp
        boost/integer_fwd.hpp
        boost/integer_traits.hpp
        boost/integer.hpp
        boost/limits.hpp
        boost/static_assert.hpp
        xi/log/filelog.h
        xi/log/baselog.cpp
        xi/log/baselog.h
        xi/log/filelog.cpp
        xi/types/monikers.h
        xi/types/types.h.other
        xi/xiTypes.h
        xiCustomLog.h
        xi/log/stdafx.cpp
        xi/log/stdafx.h
        boost/config/compiler/alignment_of_clang.hpp
        boost/config/compiler/alignment_of.hpp
        boost/config/compiler/clang.hpp
        boost/config/compiler/select_compiler_config.hpp
        boost/config/stdlib/libcpp.hpp
        boost/config/platform/macos.hpp
        boost/config/posix_features.hpp
        xi/log/windows.h
        xi/log/make.h
        xi/log/version_number.h
        xi/log/test.h
)

#set_source_files_properties(xi/db/sqlite/sqlite3.c PROPERTIES LANGUAGE CXX )

## special sublibrary for sqlite3
#add_library(sqlite3 #OBJECT
#    xi/db/sqlite/sqlite3.c
#    xi/db/sqlite/sqlite3.h
#)

add_definitions(-DSQLITE_ENABLE_RTREE)
add_definitions(-DSQLITE_ENABLE_FTS4)
add_definitions(-DSQLITE_ENABLE_FTS5)
add_definitions(-DSQLITE_ENABLE_JSON1)
add_definitions(-DSQLITE_ENABLE_RBU)
add_definitions(-DSQLITE_ENABLE_STAT4)



if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DbahnLogConverter2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET DbahnLogConverter2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(DbahnLogConverter2 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(DbahnLogConverter2
            ${PROJECT_SOURCES}
        )
    endif()
endif()

## added by xi
#target_include_directories(DbahnLogConverter2 PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
##  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/mylib>
##  $<INSTALL_INTERFACE:include/mylib>  # <prefix>/include/mylib
#)
#INCLUDE_DIRECTORIES()
target_include_directories(DbahnLogConverter2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(DbahnLogConverter2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(DbahnLogConverter2 PRIVATE Qt6::Core)

# QStringRef
target_link_libraries(DbahnLogConverter2 PRIVATE Qt6::Core5Compat)

#target_link_libraries(DbahnLogConverter2 PUBLIC sqlite3)
#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
#target_link_libraries(DbahnLogConverter2 PRIVATE Threads::Threads)

#find_package (SQLite3)
#include_directories(${SQLite3_INCLUDE_DIRS})
#target_link_libraries (DbahnLogConverter2 PRIVATE SQLite3::SQLite3)


set_target_properties(DbahnLogConverter2 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

#install(TARGETS DbahnLogConverter2
#    BUNDLE DESTINATION .
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DbahnLogConverter2)
endif()
