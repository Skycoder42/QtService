TEMPLATE = subdirs

SUBDIRS += \
	TestBaseLib \
	TestService \
	TestStandardService

unix:!android:!ios:system(pkg-config --exists libsystemd && systemctl --version): SUBDIRS += TestSystemdService

TestStandardService.depends += TestBaseLib
TestSystemdService.depends += TestBaseLib
