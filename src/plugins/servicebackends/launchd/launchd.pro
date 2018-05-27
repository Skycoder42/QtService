TARGET  = qlaunchd

QT += service service-private
QT -= gui

HEADERS += \
		launchdserviceplugin.h \
    launchdservicebackend.h

SOURCES += \
		launchdserviceplugin.cpp \
    launchdservicebackend.cpp

DISTFILES += launchd.json

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = LaunchdServicePlugin
load(qt_plugin)
