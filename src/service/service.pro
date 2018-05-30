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
    terminalserver_p.h

SOURCES += \
	service.cpp \
    servicebackend.cpp \
    servicecontrol.cpp \
    terminal.cpp \
    terminalserver.cpp

MODULE_PLUGIN_TYPES = servicebackends
load(qt_module)

win32 {
	QMAKE_TARGET_PRODUCT = "QtService"
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_COPYRIGHT = "Felix Barz"
} else:mac {
	QMAKE_TARGET_BUNDLE_PREFIX = "com.skycoder42."
}

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)

mingw: LIBS_PRIVATE += -lQt5Network -lQt5Core
