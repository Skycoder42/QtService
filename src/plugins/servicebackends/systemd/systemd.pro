TARGET  = qsystemd

QT += service service-private
QT -= gui

CONFIG += link_pkgconfig
PKGCONFIG += libsystemd

HEADERS += \
		systemdserviceplugin.h \
	systemdservicebackend.h \
    systemdcommandclient.h

SOURCES += \
		systemdserviceplugin.cpp \
	systemdservicebackend.cpp \
    systemdcommandclient.cpp

DISTFILES += systemd.json

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = SystemdServicePlugin
load(qt_plugin)
