TEMPLATE = lib
CONFIG += static

QT = core service testlib

CONFIG   += console
CONFIG   -= app_bundle

DEFINES += SRCDIR=\\\"$$_PRO_FILE_PWD_/\\\"

TARGET = testbase

HEADERS += \
	basicservicetest.h

SOURCES += \
	basicservicetest.cpp

runtarget.target = run-tests
win32: runtarget.depends += $(DESTDIR_TARGET)
else: runtarget.depends += $(TARGET)
QMAKE_EXTRA_TARGETS += runtarget
