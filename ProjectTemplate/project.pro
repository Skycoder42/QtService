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

target.path = $$[QT_INSTALL_BINS]
INSTALLS += target
@if '%{CreateSystemd}'

linux:!android {
	# install targets for systemd service files
	QMAKE_SUBSTITUTES += %{SvcSystemdName}.in

	install_svcconf.files += $$shadowed(%{SvcSystemdName})
@if '%{SocketPort}'
	install_svcconf.files += %{SvcSystemdSocketName}
@endif
	install_svcconf.CONFIG += no_check_exist
	install_svcconf.path = $$[QT_INSTALL_LIBS]/systemd/system/
	INSTALLS += install_svcconf
}
@endif
@if '%{CreateWindows}'

win32 {
	# install targets for windows service files
	QMAKE_SUBSTITUTES += %{SvcWindowsName}.in

	install_svcconf.files += $$shadowed(%{SvcWindowsName})
	install_svcconf.CONFIG += no_check_exist
	install_svcconf.path = $$[QT_INSTALL_BINS]
	INSTALLS += install_svcconf
}
@endif
@if '%{CreateLaunchd}'

macos {
	# install targets for launchd service files
	QMAKE_SUBSTITUTES += %{SvcLaunchdName}.in

	install_svcconf.files += $$shadowed(%{SvcLaunchdName})
	install_svcconf.CONFIG += no_check_exist
	install_svcconf.path = /Library/LaunchDaemons
	INSTALLS += install_svcconf
}
@endif
@if '%{CreateAndroid}'

OTHER_FILES += \\
	AndroidManifest-service.part.xml
@endif

DISTFILES += $$QMAKE_SUBSTITUTES
