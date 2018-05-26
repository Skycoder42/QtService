TEMPLATE = subdirs

SUBDIRS += service \
	plugins

android:!android-embedded: SUBDIRS += java

plugins.depends += service
java.depends += plugins

java.CONFIG += no_lrelease_target

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
