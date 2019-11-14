include(../testlib.pri)

TARGET = tst_windowsservice

SOURCES += \
		tst_windowsservice.cpp

LIBS += -ladvapi32

DEFINES+=QT_LIB_DIR=\\\"$$[QT_INSTALL_BINS]\\\"
DEFINES+=QT_PLG_DIR=\\\"$$[QT_INSTALL_PLUGINS]\\\"
