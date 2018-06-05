TEMPLATE = subdirs
QT_FOR_CONFIG += core

SUBDIRS += \
	EchoService \
	EchoControl \
	TerminalService

android: SUBDIRS += AndroidServiceTest
