TEMPLATE = app

QT = core service
CONFIG += c++14 console
CONFIG -= app_bundle

TARGET = %{TargetName}
VERSION = 1.0.0

DEFINES += QT_DEPRECATED_WARNINGS QT_ASCII_CAST_WARNINGS QT_USE_QSTRINGBUILDER
DEFINES += "TARGET=\\\\\\"$$TARGET\\\\\\""
DEFINES += "VERSION=\\\\\\"$$VERSION\\\\\\""

HEADERS += \\
	%{SvcHdrName}

SOURCES += \\
	main.cpp \\
	%{SvcSrcName}

DISTFILES += \
@if '%{CreateSystemd}'
	%{SvcSystemdName} \
@if '%{SocketPort}'
	%{SvcSystemdSocketName} \
@endif
@endif
@if '%{CreateWindows}'
	%{SvcWindowsName} \
@endif
@if '%{CreateLaunchd}'
	%{SvcLaunchdName} \
@endif
@if '%{CreateAndroid}'
	AndroidManifest-service.part.xml
@endif
