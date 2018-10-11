QT = core service qml

CXX_MODULE = service
TARGETPATH = de/skycoder42/QtService
TARGET  = declarative_service
IMPORT_VERSION = $$MODULE_VERSION_IMPORT
DEFINES += "VERSION_MAJOR=$$MODULE_VERSION_MAJOR"
DEFINES += "VERSION_MINOR=$$MODULE_VERSION_MINOR"

HEADERS += \
	qtservice_plugin.h \
	qmlservicesingleton.h

SOURCES += \
	qtservice_plugin.cpp \
	qmlservicesingleton.cpp

OTHER_FILES += qmldir

CONFIG += qmlcache
load(qml_plugin)

# overwrite the plugindump wrapper
ldpath.name = LD_LIBRARY_PATH
ldpath.value = "$$shadowed($$dirname(_QMAKE_CONF_))/lib/:$$[QT_INSTALL_LIBS]:$$(LD_LIBRARY_PATH)"
qmlpath.name = QML2_IMPORT_PATH
qmlpath.value = "$$shadowed($$dirname(_QMAKE_CONF_))/qml/:$$[QT_INSTALL_QML]:$$(QML2_IMPORT_PATH)"
QT_TOOL_ENV = ldpath qmlpath
qtPrepareTool(QMLPLUGINDUMP, qmlplugindump)
QT_TOOL_ENV =

generate_qmltypes {
	#overwrite the target deps as make target is otherwise not detected
	qmltypes.depends = ../../../qml/$$TARGETPATH/$(TARGET)

	mfirst.target = all
	mfirst.depends += qmltypes
	QMAKE_EXTRA_TARGETS += mfirst
}
