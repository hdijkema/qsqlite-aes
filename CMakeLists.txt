cmake_minimum_required(VERSION 3.16)

set(QSQLiteAES_VERSION "6.5.1")
set(PROJECT_VERSION "2.0alpha")

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

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Sql)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Sql)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Sql)

qt_prepare_standalone_project()
#qt_internal_upgrade_cmake_policies()

#qt_feature_module_begin(
#    NO_MODULE
#    PUBLIC_FILE "qtsqldrivers-config.h"
#    PRIVATE_FILE "qtsqldrivers-config_p.h"
#)
#include(configure.cmake)
#qt_feature_module_end(NO_MODULE)

#include_directories(${Qt6Sql_PRIVATE_INCLUDE_DIRS})
#add_compile_definitions(QT_BUILD_SQL_LIB)

#add_library(qsqlite-aes2 SHARED
#  qsqliteaes.cpp
#  qsqliteaes.h
#  codec.cpp
#  codec.h
#  codec_c_interface.h
#  codecext.c
#  qsql_sqlite.cpp
#  qsql_sqlite_p.h
#  qtv.h
#  shell.c
#  sqlite3.c
#  sqlite3.h
#  sqlite3ext.h
#  AES.cpp
#  AES.h
#  qsqlite-aes2_5_15_2.json
#  qsqlite-aes2_6_5_1.json
#)

#target_link_libraries(qsqlite-aes2 PRIVATE Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::SqlPrivate Qt${QT_VERSION_MAJOR}::SqlDriver)
#target_compile_definitions(qsqlite-aes2 PRIVATE QSQLITEAES2_LIBRARY)

qt_internal_add_plugin(QSQLiteAES2Plugin
    #OUTPUT_NAME qsqlite-aes
    PLUGIN_TYPE sqldrivers
    #OUTPUT_DIRECTORY c:/devel/plugins
    #INSTALL_DIRECTORY sqlite-aes
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
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::SqlPrivate
)

#### Keys ignored in scope 1:.:.:sqlite.pro:<TRUE>:
# OTHER_FILES = "sqlite.json"
# QT_FOR_CONFIG = "sqldrivers-private"

## Scopes:
#####################################################################

#qt_internal_extend_target(QSQLiteAES2Plugin
#    LIBRARIES
#        SQLite::SQLiteAES
#)

