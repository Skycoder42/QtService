#include "basicservicetest.h"
#include <QtTest>
using namespace QtService;

BasicServiceTest::BasicServiceTest(QObject *parent) :
	QObject(parent)
{}

void BasicServiceTest::initTestCase()
{
#ifdef Q_OS_LINUX
	if(!qgetenv("LD_PRELOAD").contains("Qt5Service"))
		qWarning() << "No LD_PRELOAD set - this may fail on systems with multiple version of the modules";
#endif
	init();
	control = ServiceControl::create(backend(), name(), this);
	QVERIFY(control);
	QVERIFY(control->serviceExists());
}

void BasicServiceTest::cleanupTestCase()
{
	cleanup();
}

QString BasicServiceTest::name()
{
	return QStringLiteral("testservice");
}

void BasicServiceTest::init() {}

void BasicServiceTest::cleanup() {}

void BasicServiceTest::testFeature(QtService::ServiceControl::SupportFlags flags)
{
	if(control->supportFlags().testFlag(ServiceControl::SupportsAutostart)) {
		auto meta = QMetaEnum::fromType<ServiceControl::SupportFlags>();
		QEXPECT_FAIL(qUtf8Printable(control->backend()),
					 (QByteArrayLiteral("features ") + meta.valueToKeys(static_cast<int>(flags)) + QByteArrayLiteral(" not supported by backend")).constData(),
					 Abort);
	}
}
