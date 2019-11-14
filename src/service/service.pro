TARGET = QtService

QT = core network
android: QT += androidextras

HEADERS += \
	qtservice_global.h \
	service.h \
	service_p.h \
	serviceplugin.h \
	logging_p.h \
	qtservice_helpertypes.h \
	servicebackend.h \
	servicebackend_p.h \
	servicecontrol.h \
	servicecontrol_p.h \
	terminal.h \
	terminal_p.h \
	terminalserver_p.h \
	terminalclient_p.h

SOURCES += \
	service.cpp \
	servicebackend.cpp \
	servicecontrol.cpp \
	terminal.cpp \
	terminalserver.cpp \
	terminalclient.cpp \
	serviceplugin.cpp

MODULE_PLUGIN_TYPES = servicebackends
load(qt_module)

win32 {
	QMAKE_TARGET_PRODUCT = "QtService"
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_COPYRIGHT = "Felix Barz"
} else:mac {
	QMAKE_TARGET_BUNDLE_PREFIX = "de.skycoder42."
}

QDEP_DEPENDS += \
	Skycoder42/QCtrlSignals@1.2.0 \
	Skycoder42/QPluginFactory@1.5.0 \
	Skycoder42/QConsole@1.3.1

!load(qdep):error("Failed to load qdep feature! Run 'qdep prfgen --qmake $$QMAKE_QMAKE' to create it.")
