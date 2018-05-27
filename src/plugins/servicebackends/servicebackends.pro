TEMPLATE = subdirs

SUBDIRS += standard
unix:!android:!ios:system(pkg-config --exists libsystemd): SUBDIRS += systemd
android: SUBDIRS += android
win32:!winrt: SUBDIRS += windows
macx: SUBDIRS += launchd

message("Building plugins: $$SUBDIRS")

standard.CONFIG += no_lrelease_target
systemd.CONFIG += no_lrelease_target
android.CONFIG += no_lrelease_target
windows.CONFIG += no_lrelease_target
launchd.CONFIG += no_lrelease_target

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
