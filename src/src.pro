TEMPLATE = subdirs

SUBDIRS += service \
	plugins \
	imports

android:!android-embedded: SUBDIRS += java

plugins.depends += service
imports.depends += service

QMAKE_EXTRA_TARGETS += run-tests
