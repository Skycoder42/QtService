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
DISTFILES += $$TRANSLATIONS

win32 {
	QMAKE_TARGET_PRODUCT = "QtService"
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_COPYRIGHT = "Felix Barz"
} else:mac {
	QMAKE_TARGET_BUNDLE_PREFIX = "de.skycoder42."
}

qpmx_ts_target.path = $$[QT_INSTALL_TRANSLATIONS]
qpmx_ts_target.depends += lrelease
INSTALLS += qpmx_ts_target

# extra cpp files for translations
never_true_lupdate_only {
	PLUGINS = $$files(../plugins/servicebackends/*)
	for(plugin, PLUGINS): SOURCES += $$plugin/*.h
	for(plugin, PLUGINS): SOURCES += $$plugin/*.cpp
}

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)

#replace template qm by ts
qpmx_ts_target.files -= $$OUT_PWD/$$QPMX_WORKINGDIR/qtservice_template.qm
qpmx_ts_target.files += translations/qtservice_template.ts

mingw: LIBS_PRIVATE += -lQt5Network -lQt5Core
