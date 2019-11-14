TEMPLATE = subdirs

SUBDIRS += service \
	plugins \
	imports \
	translations

android:!android-embedded: SUBDIRS += java

plugins.depends += service
imports.depends += service

QMAKE_EXTRA_TARGETS += run-tests

lupdate.target = lupdate
lupdate.CONFIG = recursive
lupdate.recurse_target = lupdate
lupdate.recurse += translations
QMAKE_EXTRA_TARGETS += lupdate
