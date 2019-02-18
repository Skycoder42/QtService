TEMPLATE = lib

TARGET = service

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

load(qt_common)

qtConfig(debug_and_release): CONFIG += debug_and_release
qtConfig(build_all): CONFIG += build_all

DESTDIR = $$MODULE_BASE_OUTDIR/lib
DLLDESTDIR = $$MODULE_BASE_OUTDIR/bin

TARGET = $$qt5LibraryTarget($$TARGET)

load(qt_installs)

message($$HEADERS)
