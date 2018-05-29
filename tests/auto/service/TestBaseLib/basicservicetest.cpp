#include "basicservicetest.h"
#include <QtTest/QtTest>
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
	control->setBlocking(true);
}

void BasicServiceTest::cleanupTestCase()
{
	if(control)
		control->stop();
	cleanup();
}

void BasicServiceTest::testStart()
{
	testFeature(ServiceControl::SupportsStatus);
	QCOMPARE(control->status(), ServiceControl::ServiceStopped);

	testFeature(ServiceControl::SupportsStart);
	QVERIFY(control->start());
	// blocking should only return after the server started, but for non blocking this may not be the case...
	if(!control->supportFlags().testFlag(ServiceControl::SupportsBlocking))
		QThread::sleep(3);

	socket = new QLocalSocket(this);
	socket->connectToServer(control->runtimeDir().absoluteFilePath(QStringLiteral("__qtservice_testservice")));
	QVERIFY(socket->waitForConnected(5000));
	stream.setDevice(socket);

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("started"));

	testFeature(ServiceControl::SupportsStatus);
	QCOMPARE(control->status(), ServiceControl::ServiceRunning);

	//test "double-start"
	QVERIFY(control->start());
	testFeature(ServiceControl::SupportsStatus);
	QCOMPARE(control->status(), ServiceControl::ServiceRunning);
}

void BasicServiceTest::testStop()
{
	testFeature(ServiceControl::SupportsStatus);
	QCOMPARE(control->status(), ServiceControl::ServiceRunning);

	testFeature(ServiceControl::SupportsStop);
	QVERIFY(control->stop());

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("stopping"));
	QVERIFY(socket->waitForDisconnected(5000));

	testFeature(ServiceControl::SupportsStatus);
	QCOMPARE(control->status(), ServiceControl::ServiceStopped);

	//test "double-stop"
	QVERIFY(control->stop());
	testFeature(ServiceControl::SupportsStatus);
	QCOMPARE(control->status(), ServiceControl::ServiceStopped);
}

QString BasicServiceTest::name()
{
	return QStringLiteral("testservice");
}

void BasicServiceTest::init() {}

void BasicServiceTest::cleanup() {}

void BasicServiceTest::testFeature(ServiceControl::SupportFlag flag)
{
	if(control->supportFlags().testFlag(flag)) {
		auto meta = QMetaEnum::fromType<ServiceControl::SupportFlags>();
		if(flag == ServiceControl::SupportsStatus) {
			QEXPECT_FAIL(qUtf8Printable(control->backend()),
						 QByteArray(QByteArrayLiteral("feature ") + meta.valueToKey(flag) + QByteArrayLiteral(" not supported by backend")).constData(),
						 Continue);
		} else {
			QEXPECT_FAIL(qUtf8Printable(control->backend()),
						 QByteArray(QByteArrayLiteral("feature ") + meta.valueToKey(flag) + QByteArrayLiteral(" not supported by backend")).constData(),
						 Abort);
		}
	}
}
