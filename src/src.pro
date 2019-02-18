TEMPLATE = subdirs

SUBDIRS += service \
	plugins \
	imports

android:!android-embedded: SUBDIRS += java

python_bindings: SUBDIRS += python

plugins.depends += service
imports.depends += service
python.depends += service

QMAKE_EXTRA_TARGETS += run-tests
