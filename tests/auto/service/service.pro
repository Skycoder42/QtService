TEMPLATE = subdirs

SUBDIRS += \
	TestBaseLib \
	TestService \
	TestStandardService

unix:!android:!ios:system(pkg-config --exists libsystemd && systemctl --version): SUBDIRS += TestSystemdService
win32: SUBDIRS += TestWindowsService

TestStandardService.depends += TestBaseLib
TestSystemdService.depends += TestBaseLib
TestWindowsService.depends += TestBaseLib
