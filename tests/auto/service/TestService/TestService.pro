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
