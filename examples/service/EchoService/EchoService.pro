TEMPLATE = app

QT += service
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TARGET = echoservice

HEADERS += \
	echoservice.h

SOURCES += \
	main.cpp \
	echoservice.cpp

QMAKE_SUBSTITUTES += \
	echoservice.service.in \
	scinstall.bat.in \
	echoservice.plist.in

DISTFILES += $$QMAKE_SUBSTITUTES \
	echoservice.socket

target.path = $$[QT_INSTALL_EXAMPLES]/service/EchoService
!install_ok: INSTALLS += target

win32: install_svcconf.files += $$shadowed(scinstall.bat)
else:macos: install_svcconf.files += $$shadowed(echoservice.plist)
else:linux:!android: install_svcconf.files += $$shadowed(echoservice.service) echoservice.socket
install_svcconf.CONFIG += no_check_exist
install_svcconf.path = $$[QT_INSTALL_EXAMPLES]/service/EchoService
!install_ok: INSTALLS += install_svcconf
