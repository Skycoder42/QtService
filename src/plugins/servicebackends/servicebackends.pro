TEMPLATE = subdirs

SUBDIRS += standard
unix:!android:!ios:system(pkg-config --exists libsystemd): SUBDIRS += systemd

standard.CONFIG += no_lrelease_target
systemd.CONFIG += no_lrelease_target

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
