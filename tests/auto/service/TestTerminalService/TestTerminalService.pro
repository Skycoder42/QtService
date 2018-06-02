TEMPLATE = app

QT = core service testlib

CONFIG   += console
CONFIG   -= app_bundle

TARGET = tst_terminalservice

DEFINES += SRCDIR=\\\"$$_PRO_FILE_PWD_/\\\"

SOURCES += \
		tst_terminalservice.cpp

mac: QMAKE_LFLAGS += '-Wl,-rpath,\'$$OUT_PWD/../../../../lib\''
