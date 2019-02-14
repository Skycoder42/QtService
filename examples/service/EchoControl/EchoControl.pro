TEMPLATE = app

TARGET = EchoControl
QT += core gui widgets service

SOURCES += \
	main.cpp \
	controlwidget.cpp

HEADERS += \
	controlwidget.h

FORMS += \
	controlwidget.ui

target.path = $$[QT_INSTALL_EXAMPLES]/service/$$TARGET
INSTALLS += target
