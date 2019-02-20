load(qt_build_paths)

defineReplace(pyside2_config) {
	win32: command = python.exe
	else: command = python3
	command += $$system_quote($$PWD/pyside2_config.py) $$1
	result = $$system($$command)
	return($$result)
}

defineReplace(python_config) {
	win32: command = python.exe
	command += $$system_quote($$PWD/config.py) $$1
	result = $$system($$command)
	return($$result)
}

isEmpty(PYSIDE2_PREFIX) {
	QT_TOOL.shiboken2.binary = $$system_path($$pyside2_config(--shiboken2-generator-path)/shiboken2)
	qtPrepareTool(SHIBOKEN2, shiboken2)
	isEmpty(SHIBOKEN2_TYPESYSTEM_PATH): SHIBOKEN2_TYPESYSTEM_PATH = $$pyside2_config(--pyside2-path)/typesystems

	INCLUDEPATH += \
		$$pyside2_config(--shiboken2-generator-include-path) \
		$$pyside2_config(--pyside2-include-path)
	for(incpath, $$list($$pyside2_config(--pyside2-include-path))): \
		for(module, SHIBOKEN_MODULE_DEPENDS): \
			INCLUDEPATH += $$shell_quote($$incpath/$$module)
} else {
	QT_TOOL.shiboken2.binary = $$system_path($$PYSIDE2_PREFIX/bin/shiboken2)
	qtPrepareTool(SHIBOKEN2, shiboken2)
	isEmpty(SHIBOKEN2_TYPESYSTEM_PATH): SHIBOKEN2_TYPESYSTEM_PATH = $$PYSIDE2_PREFIX/share/PySide2/typesystems

	INCLUDEPATH += \
		$$PYSIDE2_PREFIX/include/shiboken2 \
		$$PYSIDE2_PREFIX/include/PySide2
	for(module, SHIBOKEN_MODULE_DEPENDS): INCLUDEPATH += $$shell_quote($$PYSIDE2_PREFIX/include/PySide2/$$module)
}

INCLUDEPATH += $$pyside2_config(--python-include-path)

LIBS += \
	$$pyside2_config(--shiboken2-module-qmake-lflags) \
	$$pyside2_config(--pyside2-qmake-lflags) \
	$$pyside2_config(--python-link-flags-qmake)

isEmpty(SHIBOKEN2_DIR): SHIBOKEN2_DIR = .
debug_and_release {
	CONFIG(debug, debug|release): SHIBOKEN2_DIR = $$SHIBOKEN2_DIR/debug
	CONFIG(release, debug|release): SHIBOKEN2_DIR = $$SHIBOKEN2_DIR/release
}
SHIBOKEN2_SRC_DIR = $$SHIBOKEN2_DIR/PySide2/$$SHIBOKEN_MODULE_TARGET

isEmpty(SHIBOKEN2_CORE_FLAGS) {
	 SHIBOKEN2_CORE_FLAGS = \
		--generator-set=shiboken \
		--enable-parent-ctor-heuristic \
		--enable-return-value-heuristic \
		--use-isnull-as-nb_nonzero
		--avoid-protected-hack
	qt: SHIBOKEN2_CORE_FLAGS += --enable-pyside-extensions

	c++17: SHIBOKEN2_CORE_FLAGS += --language-level=c++17
	else:c++14: SHIBOKEN2_CORE_FLAGS += --language-level=c++14
	else:c++11: SHIBOKEN2_CORE_FLAGS += --language-level=c++11
}

# shiboken command
{
	__SHIBOKEN_ARGS = $$SHIBOKEN2_CORE_FLAGS $$SHIBOKEN2_FLAGS
	__SHIBOKEN_ARGS += $$shell_quote(-I$$[QT_INSTALL_HEADERS])
	for(module, SHIBOKEN_MODULE_DEPENDS): __SHIBOKEN_ARGS += $$shell_quote(-I$$[QT_INSTALL_HEADERS]/$$module)
	__SHIBOKEN_ARGS += $$shell_quote(-I$$MODULE_BASE_OUTDIR/include)
	__SHIBOKEN_ARGS += $$shell_quote(-I$$_PRO_FILE_PWD_)
	for(incpath, INCLUDEPATH) __SHIBOKEN_ARGS += $$shell_quote(-I$$incpath)
	qtConfig(framework) {
		__SHIBOKEN_ARGS += $$shell_quote(-F$$[QT_INSTALL_LIBS])
		for(lib, LIBS) {
			prefix = $$str_member($$lib, 0, 1)
			equals(prefix, "-F"): __SHIBOKEN_ARGS += $$shell_quote($$lib)
		}
	}
	for(tspath, SHIBOKEN2_TYPESYSTEM_PATH): __SHIBOKEN_ARGS += $$shell_quote(-T$$tspath)
	__SHIBOKEN_ARGS += $$shell_quote(--output-directory=$$SHIBOKEN2_DIR)
	__SHIBOKEN_ARGS += $$shell_quote($$MODULE_BASE_OUTDIR/include/$$SHIBOKEN_MODULE_TARGET/$$SHIBOKEN_MODULE_TARGET)

	shiboken2_c.name = shiboken ${QMAKE_FILE_IN}.xml
	shiboken2_c.input = SHIBOKEN_BINDINGS
	shiboken2_c.variable_out = GENERATED_SOURCES
	shiboken2_c.commands = $$SHIBOKEN2 $$__SHIBOKEN_ARGS ${QMAKE_FILE_IN}
	shiboken2_c.output = $$SHIBOKEN2_SRC_DIR/$$lower($$SHIBOKEN_MODULE_TARGET)_module_wrapper.cpp
	shiboken2_c.depends += $$SHIBOKEN2_EXE
	shiboken2_c.dependency_type = TYPE_C
	QMAKE_EXTRA_COMPILERS += shiboken2_c

	shiboken2_h_c.name = shiboken headers ${QMAKE_FILE_IN}
	shiboken2_h_c.input = SHIBOKEN_WRAPPERS
	shiboken2_h_c.variable_out = HEADERS
	shiboken2_h_c.commands = $$escape_expand(\\n) # force creation of rule
	shiboken2_h_c.output = $$SHIBOKEN2_SRC_DIR/${QMAKE_FILE_BASE}_wrapper.h
	shiboken2_h_c.CONFIG += explicit_dependencies no_dependencies
	shiboken2_h_c.depends += $$SHIBOKEN2_SRC_DIR/$$lower($$SHIBOKEN_MODULE_TARGET)_module_wrapper.cpp
	shiboken2_h_c.dependency_type = TYPE_C
	QMAKE_EXTRA_COMPILERS += shiboken2_h_c

	shiboken2_cpp_c.name = shiboken sources ${QMAKE_FILE_IN}
	shiboken2_cpp_c.input = SHIBOKEN_WRAPPERS
	shiboken2_cpp_c.variable_out = GENERATED_SOURCES
	shiboken2_cpp_c.commands = $$escape_expand(\\n) # force creation of rule
	shiboken2_cpp_c.output = $$SHIBOKEN2_SRC_DIR/${QMAKE_FILE_BASE}_wrapper.cpp
	shiboken2_cpp_c.CONFIG += explicit_dependencies no_dependencies
	shiboken2_cpp_c.depends += $$SHIBOKEN2_SRC_DIR/$$lower($$SHIBOKEN_MODULE_TARGET)_module_wrapper.cpp
	shiboken2_cpp_c.dependency_type = TYPE_C
	QMAKE_EXTRA_COMPILERS += shiboken2_cpp_c

	for(wrapper, SHIBOKEN_WRAPPERS): QMAKE_EXTRA_TARGETS += $$wrapper

	HEADERS += $$shadowed($$SHIBOKEN2_SRC_DIR/pyside2_$$lower($$SHIBOKEN_MODULE_TARGET)_python.h)
}

load(qt_common)

QT =
for(qt_mod, $$list($$SHIBOKEN_MODULE_TARGET $$SHIBOKEN_MODULE_DEPENDS)): \
	QT += $$lower($$str_member($$qt_mod, 2, -1))

CONFIG += unversioned_soname unversioned_libname plugin no_plugin_name_prefix
qtConfig(debug_and_release): CONFIG += debug_and_release
qtConfig(build_all): CONFIG += build_all

darwin: QMAKE_CXXFLAGS += -undefined dynamic_lookup

DESTDIR = $$MODULE_BASE_OUTDIR/lib/PySide2
DLLDESTDIR = $$MODULE_BASE_OUTDIR/bin/PySide2

TARGET = "$${SHIBOKEN_MODULE_TARGET}"
QMAKE_EXTENSION_SHLIB = $$str_member($$python_config(suffix), 1, -1)

DISTFILES += \
	$$PWD/pyside2_config.py \
	$$PWD/setup.py \
	$$PWD/config.py
