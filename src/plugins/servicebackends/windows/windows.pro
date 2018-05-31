TARGET  = qwindows

QT += service service-private
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
