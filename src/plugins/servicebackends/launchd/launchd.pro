TARGET  = qlaunchd

QT += service
QT -= gui

HEADERS += \
		launchdserviceplugin.h \
	launchdservicebackend.h \
	launchdservicecontrol.h

SOURCES += \
		launchdserviceplugin.cpp \
	launchdservicebackend.cpp \
	launchdservicecontrol.cpp

DISTFILES += launchd.json

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = LaunchdServicePlugin
load(qt_plugin)
