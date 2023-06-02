cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION_MAJOR "6")
set(PROJECT_VERSION_MINOR "5")
set(PROJECT_VERSION_PATCH "1")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(SQLiteAES_VERSION ${PROJECT_VERSION})

message(NOTICE PROJECT_VERSION=${PROJECT_VERSION})

project(qsqlite-aes2 LANGUAGES C CXX)
project(QSQLiteAES2DriverPlugins
    VERSION "${SQLiteAES_VERSION}"
    DESCRIPTION "Qt6 SQL driver plugins for SQLite, with AES encryption"
    HOMEPAGE_URL "https://github.com/hdijkema/qsqlite-aes"
    LANGUAGES CXX C
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Sql)

qt_prepare_standalone_project()
qt_internal_upgrade_cmake_policies()

if(APPLE)
    message(NOTICE "MacOSX build of SQLite AES Driver. Setting extra variables for this target")

    set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)
    message(NOTICE "MacOSX Deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")

    set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)
    message(NOTICE "MacOSX Architectures: ${CMAKE_OSX_ARCHITECTURES}")
    add_definitions(-DPACKAGE_NAME="sqlite" -DPACKAGE_TARNAME="sqlite" -DPACKAGE_VERSION="3.29.0")
    add_definitions(-DPACKAGE_STRING="sqlite 3.29.0" -DPACKAGE_BUGREPORT="http://www.sqlite.org")
    add_definitions(-DPACKAGE_URL="" -DPACKAGE="sqlite" -DVERSION="3.29.0" -DSTDC_HEADERS=1)
    add_definitions(-DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1)
    add_definitions(-DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1)
    add_definitions(-DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=".libs/" -DHAVE_FDATASYNC=1)
    add_definitions(-DHAVE_USLEEP=1 -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DHAVE_DECL_STRERROR_R=1)
    add_definitions(-DHAVE_STRERROR_R=1 -DHAVE_EDITLINE_READLINE_H=1 -DHAVE_READLINE_READLINE_H=1)
    add_definitions(-DHAVE_READLINE=1 -DHAVE_ZLIB_H=1 -I.    -D_REENTRANT=1 -DSQLITE_THREADSAFE=1)
    add_definitions(-DSQLITE_HAVE_ZLIB  -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_RTREE)
    link_libraries(z readline)
endif(APPLE)

qt_internal_add_plugin(QSQLiteAES2Plugin
    OUTPUT_NAME qsqlite-aes_${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}
    PLUGIN_TYPE sqldrivers
    SOURCES
      qsqliteaes.cpp
      qsqliteaes.h
      codec.cpp
      codec.h
      codec_c_interface.h
      codecext.c
      qsql_sqlite.cpp
      qsql_sqlite_p.h
      qtv.h
      shell.c
      sqlite3.c
      sqlite3.h
      sqlite3ext.h
      AES.cpp
      AES.h
      qsqlite-aes2_5_15_2.json
      qsqlite-aes2_6_5_1.json
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        SQLITE_ENABLE_COLUMN_METADATA
        SQLITE_ENABLE_FTS5
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::SqlPrivate
)


