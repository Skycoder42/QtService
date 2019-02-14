TEMPLATE = app

QT += service
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TARGET = echoservice

HEADERS += \
	echoservice.h

SOURCES += \
	main.cpp \
	echoservice.cpp

DISTFILES += \
	echoservice.service \
	echoservice.socket \
	scinstall.bat \
	echoservice.plist

target.path = $$[QT_INSTALL_EXAMPLES]/service/EchoService
INSTALLS += target
