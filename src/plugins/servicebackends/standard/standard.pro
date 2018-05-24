TARGET  = qstandard

QT += service service-private
QT -= gui

HEADERS += \
	standardservicebackend.h \
	standardserviceplugin.h

SOURCES += \
	standardservicebackend.cpp \
	standardserviceplugin.cpp

DISTFILES += standard.json

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = StandardServicePlugin
load(qt_plugin)
