TARGET  = qstandard

QT += service
QT -= gui

HEADERS += \
	standardserviceplugin.h \
	standardservicebackend.h \
	standardservicecontrol.h

SOURCES += \
	standardserviceplugin.cpp \
	standardservicebackend.cpp \
	standardservicecontrol.cpp

DISTFILES += standard.json

win32: LIBS += -lkernel32

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = StandardServicePlugin
load(qt_plugin)

DISTFILES += standard.json
json_target.target = $$OBJECTS_DIR/moc_standardserviceplugin.o
json_target.depends += $$PWD/standard.json
QMAKE_EXTRA_TARGETS += json_target
