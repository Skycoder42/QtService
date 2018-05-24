TARGET  = qdummy

QT += service
QT -= gui

HEADERS += \
	dummyserviceplugin.h \
    dummyservicebackend.h

SOURCES += \
	dummyserviceplugin.cpp \
    dummyservicebackend.cpp

DISTFILES += dummy.json

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = DummyServicePlugin
load(qt_plugin)
