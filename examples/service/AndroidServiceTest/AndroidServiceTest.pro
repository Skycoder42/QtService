TEMPLATE = app

QT += quick service androidextras
CONFIG += c++14

HEADERS += \
	testservice.h \
	controlhelper.h

SOURCES += \
	main.cpp \
	testservice.cpp \
	controlhelper.cpp

RESOURCES += qml.qrc

DISTFILES += \
	android/AndroidManifest.xml \
	android/res/values/libs.xml \
	android/build.gradle \
	android/src/de/skycoder42/qtservice/test/TestServiceHelper.java

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

target.path = $$[QT_INSTALL_EXAMPLES]/service/$$TARGET
!install_ok: INSTALLS += target
