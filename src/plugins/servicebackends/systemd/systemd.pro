TARGET  = qsystemd

QT += service dbus
QT -= gui

CONFIG += link_pkgconfig
PKGCONFIG += libsystemd

HEADERS += \
	systemdserviceplugin.h \
	systemdservicebackend.h \
	systemdservicecontrol.h

SOURCES += \
	systemdserviceplugin.cpp \
	systemdservicebackend.cpp \
	systemdservicecontrol.cpp

DBUS_INTERFACES += de.skycoder42.QtService.ServicePlugin.systemd.xml
DBUS_ADAPTORS += $$DBUS_INTERFACES

DISTFILES += \
	systemd.json

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = SystemdServicePlugin
load(qt_plugin)

DISTFILES += systemd.json
json_target.target = $$OBJECTS_DIR/moc_systemdserviceplugin.o
json_target.depends += $$PWD/systemd.json
QMAKE_EXTRA_TARGETS += json_target
