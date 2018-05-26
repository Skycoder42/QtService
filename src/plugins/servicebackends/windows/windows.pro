TARGET  = qwindows

QT += service service-private
QT -= gui

HEADERS += \
	windowsserviceplugin.h \
	windowsservicebackend.h

SOURCES += \
	windowsserviceplugin.cpp \
	windowsservicebackend.cpp

DISTFILES += windows.json

LIBS += -ladvapi32

PLUGIN_TYPE = servicebackends
PLUGIN_EXTENDS = service
PLUGIN_CLASS_NAME = WindowsServicePlugin
load(qt_plugin)
