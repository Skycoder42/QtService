TEMPLATE = subdirs

SUBDIRS += \
	TestBaseLib \
	TestService \
	TestStandardService \
	TestTerminalService

unix:!android:!ios:system(pkg-config --exists libsystemd && systemctl --version): SUBDIRS += TestSystemdService
win32: SUBDIRS += TestWindowsService
macx: SUBDIRS += TestLaunchdService

TestStandardService.depends += TestService
TestTerminalService.depends += TestBaseLib TestService
TestSystemdService.depends += TestBaseLib TestService
TestWindowsService.depends += TestBaseLib TestService
TestLaunchdService.depends += TestBaseLib TestService

prepareRecursiveTarget(run-tests)
QMAKE_EXTRA_TARGETS += run-tests
