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

TRANSLATIONS += translations/qtservice_de.ts \
	translations/qtservice_template.ts

CONFIG += lrelease
QM_FILES_INSTALL_PATH = $$[QT_INSTALL_TRANSLATIONS]

win32 {
	QMAKE_TARGET_PRODUCT = "QtService"
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_COPYRIGHT = "Felix Barz"
} else:mac {
	QMAKE_TARGET_BUNDLE_PREFIX = "de.skycoder42."
}

QDEP_DEPENDS += Skycoder42/QCtrlSignals Skycoder42/QPluginFactory Skycoder42/QConsole

# extra cpp files for translations
never_true_lupdate_only {
	PLUGINS = $$files(../plugins/servicebackends/*)
	for(plugin, PLUGINS): SOURCES += $$plugin/*.h
	for(plugin, PLUGINS): SOURCES += $$plugin/*.cpp
}

!load(qdep):error("Failed to load qdep feature! Run 'qdep prfgen --qmake $$QMAKE_QMAKE' to create it.")

#replace template qm by ts
QM_FILES -= $$__qdep_lrelease_real_dir/qtservice_template.qm
QM_FILES += translations/qtservice_template.ts

