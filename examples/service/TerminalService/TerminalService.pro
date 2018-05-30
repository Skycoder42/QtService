TEMPLATE = app

QT += service
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TARGET = terminalservice

HEADERS += \
    terminalservice.h

SOURCES += \
	main.cpp \
    terminalservice.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/service/TerminalService
INSTALLS += target

#add lib dir to rpath
mac: QMAKE_LFLAGS += '-Wl,-rpath,\'$$OUT_PWD/../../../lib\''
