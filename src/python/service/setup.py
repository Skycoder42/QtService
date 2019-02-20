from distutils.core import setup, Extension

module1 = Extension('PySide2.QtService', language='c++', sources=[
						'PySide2/QtService/qtservice_module_wrapper.cpp',
						'PySide2/QtService/qtservice_service_wrapper.cpp',
						'PySide2/QtService/qtservice_servicebackend_wrapper.cpp',
						'PySide2/QtService/qtservice_servicecontrol_wrapper.cpp',
						'PySide2/QtService/qtservice_terminal_awaitable_wrapper.cpp',
						'PySide2/QtService/qtservice_terminal_wrapper.cpp',
						'PySide2/QtService/qtservice_wrapper.cpp'
					], depends=[
						'PySide2/QtService/pyside2_qtservice_python.h',
						'PySide2/QtService/qtservice_service_wrapper.h',
						'PySide2/QtService/qtservice_servicebackend_wrapper.h',
						'PySide2/QtService/qtservice_servicecontrol_wrapper.h',
						'PySide2/QtService/qtservice_terminal_awaitable_wrapper.h',
						'PySide2/QtService/qtservice_terminal_wrapper.h',
						'PySide2/QtService/qtservice_wrapper.h'
					], define_macros=[
						('QT_NO_NARROWING_CONVERSIONS_IN_CONNECT', None),
						('QT_SERVICE_LIB', None),
						('QT_NETWORK_LIB', None),
						('QT_CORE_LIB', None)
					], include_dirs=[
						'../../../include',
						'../../../include/QtService',
						'/usr/include/shiboken2',
						'/usr/include/PySide2',
						'/usr/include/PySide2/QtCore',
						'/usr/include/PySide2/QtNetwork',
						'/usr/include/qt',
						'/usr/include/qt/QtCore',
						'/usr/include/qt/QtNetwork'
					], library_dirs=[
						'/home/sky/Programming/QtLibraries/build-qtservice-System_Qt_5_12_0-Debug/lib'
						'/usr/lib/python3.7/site-packages/shiboken2',
						'/usr/lib/python3.7/site-packages/PySide2'
						'/usr/lib'
					], libraries=[
						'Qt5Service',
						'Qt5Network',
						'Qt5Core'
					])

setup(name='PySide2.QtService',
	  version='1.1.2',
	  description='This is a demo package',
	  ext_modules=[module1])
