TEMPLATE = app

QT += service
QT -= gui

CONFIG += console
CONFIG -= app_bundle

HEADERS += \
	echoservice.h

SOURCES += \
	main.cpp \
	echoservice.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/service/$$TARGET
INSTALLS += target

#add lib dir to rpath
mac: QMAKE_LFLAGS += '-Wl,-rpath,\'$$OUT_PWD/../../../lib\''
