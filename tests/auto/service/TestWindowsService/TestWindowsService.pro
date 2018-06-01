include(../testlib.pri)

TARGET = tst_windowsservice

SOURCES += \
		tst_windowsservice.cpp

LIBS += -ladvapi32
