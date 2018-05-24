TEMPLATE = subdirs

QT_FOR_CONFIG += core
SUBDIRS += \
	servicebackends

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
