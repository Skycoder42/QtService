TEMPLATE = aux

QDEP_LUPDATE_INPUTS += $$PWD/../service
QDEP_LUPDATE_INPUTS += $$PWD/../plugins
QDEP_LUPDATE_INPUTS += $$PWD/../imports
QDEP_LUPDATE_INPUTS += $$PWD/../java

TRANSLATIONS += \
	qtservice_de.ts \
	qtservice_template.ts

CONFIG += lrelease
QM_FILES_INSTALL_PATH = $$[QT_INSTALL_TRANSLATIONS]

QDEP_DEPENDS += \
	Skycoder42/QCtrlSignals@1.2.0 \
	Skycoder42/QPluginFactory@1.5.0 \
	Skycoder42/QConsole@1.3.1

!load(qdep):error("Failed to load qdep feature! Run 'qdep prfgen --qmake $$QMAKE_QMAKE' to create it.")

#replace template qm by ts
QM_FILES -= $$__qdep_lrelease_real_dir/qtservice_template.qm
QM_FILES += translations/qtservice_template.ts

HEADERS =
SOURCES =
GENERATED_SOURCES =
OBJECTIVE_SOURCES =
RESOURCES =
