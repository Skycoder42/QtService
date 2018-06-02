TEMPLATE = subdirs

SUBDIRS += \
	TestBaseLib \
	TestService \
	TestStandardService \
    TestTerminalService

unix:!android:!ios:system(pkg-config --exists libsystemd && systemctl --version): SUBDIRS += TestSystemdService
win32: SUBDIRS += TestWindowsService
macx: SUBDIRS += TestLaunchdService

TestStandardService.depends += TestBaseLib
TestSystemdService.depends += TestBaseLib
TestWindowsService.depends += TestBaseLib
TestLaunchdService.depends += TestBaseLib
