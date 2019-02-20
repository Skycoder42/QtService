TEMPLATE = lib

load(qt_build_paths)

SHIBOKEN_MODULE_TARGET = QtService
SHIBOKEN_MODULE_DEPENDS = QtCore QtNetwork

SHIBOKEN_BINDINGS += \
	typesystem_service.xml

SHIBOKEN_WRAPPERS += \
	qtservice \
	qtservice_service \
	qtservice_servicebackend \
	qtservice_servicecontrol \
	qtservice_terminal \
	qtservice_terminal_awaitable

PYSIDE2_PREFIX = /usr

#SHIBOKEN2_FLAGS += --debug-level=full --no-suppress-warnings

include(shiboken.pri)

message(HEADERS $$HEADERS)
message(INCLUDEPATH $$INCLUDEPATH)
message(DEFINES $$DEFINES)
message(LIBS $$LIBS)
