TEMPLATE = subdirs

SUBDIRS += service \
	plugins \
	imports

android:!android-embedded: SUBDIRS += java

plugins.depends += service
imports.depends += service

java.CONFIG += no_lrelease_target
plugins.CONFIG += no_lrelease_target
imports.CONFIG += no_lrelease_target

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
