TEMPLATE = subdirs

SUBDIRS += service \
	plugins

android:!android-embedded: SUBDIRS += java

plugins.depends += service
java.depends += plugins

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
