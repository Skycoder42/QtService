TARGET  = qstandard

QT += service service-private
QT -= gui

HEADERS += \
	standardservicebackend.h \
	standardserviceplugin.h \
	standardservicecontrol.h

SOURCES += \
	standardservicebackend.cpp \
	standardserviceplugin.cpp \
	standardservicecontrol.cpp

DISTFILES += standard.json

win32: LIBS += kernel32

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = StandardServicePlugin
load(qt_plugin)
