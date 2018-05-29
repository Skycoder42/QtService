TEMPLATE = subdirs

SUBDIRS += \
	TestBaseLib \
	TestService \
	TestStandardService

TestStandardService.depends += TestBaseLib
