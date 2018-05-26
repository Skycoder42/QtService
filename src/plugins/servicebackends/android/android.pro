TARGET  = qandroid

QT += service service-private androidextras
QT -= gui

HEADERS += \
	androidserviceplugin.h \
	androidservicebackend.h

SOURCES += \
	androidserviceplugin.cpp \
	androidservicebackend.cpp

DISTFILES += android.json

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = AndroidServicePlugin
load(qt_plugin)
