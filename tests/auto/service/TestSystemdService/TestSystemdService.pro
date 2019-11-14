include(../testlib.pri)

TARGET = tst_systemdservice

SOURCES += \
	tst_systemdservice.cpp

DISTFILES += \
	testservice.service \
	testservice.socket
