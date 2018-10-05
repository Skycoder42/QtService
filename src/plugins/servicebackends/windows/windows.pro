TARGET  = qwindows

QT += service
QT -= gui

HEADERS += \
	windowsserviceplugin.h \
	windowsservicebackend.h \
	windowsservicecontrol.h

SOURCES += \
	windowsserviceplugin.cpp \
	windowsservicebackend.cpp \
	windowsservicecontrol.cpp

DISTFILES += windows.json

LIBS += -ladvapi32

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = WindowsServicePlugin
load(qt_plugin)

DISTFILES += windows.json
json_target.target = $$OBJECTS_DIR/moc_windowsserviceplugin.o
json_target.depends += $$PWD/windows.json
QMAKE_EXTRA_TARGETS += json_target
