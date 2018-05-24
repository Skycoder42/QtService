TEMPLATE = subdirs

SUBDIRS += \
	standard

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
