TEMPLATE = subdirs

SUBDIRS += standard
unix:!android:!ios:packagesExist(libsystemd): SUBDIRS += systemd
android: SUBDIRS += android
win32:!winrt: SUBDIRS += windows
macx: SUBDIRS += launchd

message("Building plugins: $$SUBDIRS")

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
