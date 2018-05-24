TEMPLATE = subdirs

SUBDIRS += \
	dummy

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
