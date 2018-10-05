TEMPLATE = app

QT = core service

CONFIG += console
CONFIG -= app_bundle

TARGET = testservice

HEADERS += \
	testservice.h

SOURCES += \
	main.cpp \
	testservice.cpp

runtarget.target = run-tests
win32: runtarget.depends += $(DESTDIR_TARGET)
else: runtarget.depends += $(TARGET)
QMAKE_EXTRA_TARGETS += runtarget
