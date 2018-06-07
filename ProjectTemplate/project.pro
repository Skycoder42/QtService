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

@if '%{CreateSystemd}'
DISTFILES += %{SvcSystemdName}
@if '%{SocketPort}'
DISTFILES += %{SvcSystemdSocketName}
@endif
@endif
@if '%{CreateWindows}'
DISTFILES += %{SvcWindowsName}
@endif
@if '%{CreateLaunchd}'
DISTFILES += %{SvcLaunchdName}
@endif
@if '%{CreateAndroid}'
DISTFILES += AndroidManifest-service.part.xml
@endif
