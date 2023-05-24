###################################################################
# This version works until Qt 6.4.3.
# From Qt 6.5 on, we need to use CMake.
###################################################################

TARGET = qsqlite-aes_"$$QT_MAJOR_VERSION"_"$$QT_MINOR_VERSION"_"$$QT_PATCH_VERSION"
message($$TARGET)
QT_FOR_CONFIG += sqldrivers-private

mac: message(Qt Deployment Target: $$QMAKE_MACOSX_DEPLOYMENT_TARGET)
mac: QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

HEADERS += $$PWD/qsql_sqlite_p.h \
    qtv.h \
    sqlite3.h \
    codec.h \
    codec_c_interface.h \
    AES.h
SOURCES += $$PWD/qsql_sqlite.cpp $$PWD/smain.cpp \
    sqlite3.c \
    codec.cpp \
    AES.cpp \
    codecext.c

OTHER_FILES +=

PLUGIN_CLASS_NAME = QSQLiteAESPlugin

# follows contents of include(../qsqldriverbase.pri)
QT  = core core-private sql-private

PLUGIN_TYPE = sqldrivers
load(qt_plugin)

#QMAKE_CFLAGS += -DJSONFILE=$$TARGET
DEFINES += "JSONFILE=\"$$TARGET\""

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII SQLITE_ENABLE_COLUMN_METADATA SQLITE_ENABLE_FTS5

mac: QMAKE_CFLAGS += -DPACKAGE_NAME=\"sqlite\" -DPACKAGE_TARNAME=\"sqlite\" -DPACKAGE_VERSION=\"3.29.0\"
mac: QMAKE_CFLAGS += -DPACKAGE_STRING=\"sqlite\ 3.29.0\" -DPACKAGE_BUGREPORT=\"http://www.sqlite.org\"
mac: QMAKE_CFLAGS += -DPACKAGE_URL=\"\" -DPACKAGE=\"sqlite\" -DVERSION=\"3.29.0\" -DSTDC_HEADERS=1
mac: QMAKE_CFLAGS += -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1
mac: QMAKE_CFLAGS += -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1
mac: QMAKE_CFLAGS += -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DHAVE_FDATASYNC=1
mac: QMAKE_CFLAGS += -DHAVE_USLEEP=1 -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DHAVE_DECL_STRERROR_R=1
mac: QMAKE_CFLAGS += -DHAVE_STRERROR_R=1 -DHAVE_EDITLINE_READLINE_H=1 -DHAVE_READLINE_READLINE_H=1
mac: QMAKE_CFLAGS += -DHAVE_READLINE=1 -DHAVE_ZLIB_H=1 -I.    -D_REENTRANT=1 -DSQLITE_THREADSAFE=1
mac: QMAKE_CFLAGS += -DSQLITE_HAVE_ZLIB  -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_RTREE

linux: QMAKE_CFLAGS += -DPACKAGE_NAME=\"sqlite\" -DPACKAGE_TARNAME=\"sqlite\" -DPACKAGE_VERSION=\"3.29.0\"
linux: QMAKE_CFLAGS += -DPACKAGE_STRING=\"sqlite\ 3.29.0\" -DPACKAGE_BUGREPORT=\"http://www.sqlite.org\"
linux: QMAKE_CFLAGS += -DPACKAGE_URL=\"\" -DPACKAGE=\"sqlite\" -DVERSION=\"3.29.0\" -DSTDC_HEADERS=1
linux: QMAKE_CFLAGS += -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1
linux: QMAKE_CFLAGS += -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1
linux: QMAKE_CFLAGS += -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DHAVE_FDATASYNC=1
linux: QMAKE_CFLAGS += -DHAVE_USLEEP=1 -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DHAVE_DECL_STRERROR_R=1
linux: QMAKE_CFLAGS += -DHAVE_STRERROR_R=1 -DHAVE_EDITLINE_READLINE_H=1 -DHAVE_READLINE_READLINE_H=1
linux: QMAKE_CFLAGS += -DHAVE_READLINE=1 -DHAVE_ZLIB_H=1 -I.    -D_REENTRANT=1 -DSQLITE_THREADSAFE=1
linux: QMAKE_CFLAGS += -DSQLITE_HAVE_ZLIB  -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_RTREE

linux:LIBS += -ldl

DISTFILES += \
    .qmake.conf \
    LICENSE \
    README \
    README.md \
    sqlite-aes_5_12_3.json \
    sqlite-aes_5_12_5.json \
    sqlite-aes_5_13_1.json \
    sqlite-aes_5_14_0.json \
    sqlite-aes_5_14_2.json \
    sqlite-aes_5_15_2.json \
    sqlite-aes_6_2_3.json \
    sqlite-aes_6_2_4.json \
    sqlite-aes_6_3_1.json \
    sqlite-aes_6_4_0.json \
    sqlite-aes_6_4_3.json \
    sqlite-sodium.json \
    sqlite-aes_5_12_3.json
