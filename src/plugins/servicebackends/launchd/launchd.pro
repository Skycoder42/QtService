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

DISTFILES += launchd.json
json_target.target = $$OBJECTS_DIR/moc_launchdserviceplugin.o
json_target.depends += $$PWD/launchd.json
QMAKE_EXTRA_TARGETS += json_target
