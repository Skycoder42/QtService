TEMPLATE = subdirs

SUBDIRS += \
	TestBasicService \
	TestService \
	TestStandardService

TestStandardService.depends += TestBasicService
