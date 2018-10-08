#include "basicservicetest.h"
#include <QtTest/QtTest>
#include <qipcpointer.h>
using namespace QtService;

struct Data
{
	struct OpInfo {
		QtService::Service::CommandResult mode;
	};
	QIpcPointer<OpInfo> opInfo;
};

BasicServiceTest::BasicServiceTest(QObject *parent) :
	QObject{parent},
	d{new Data{}}
{}

BasicServiceTest::~BasicServiceTest() = default;

void BasicServiceTest::initTestCase()
{
	d->opInfo = QIpcPointer<Data::OpInfo>::create(QStringLiteral("de.skycoder42.qtservice.tests.testservice.sharedmem"));
	QVERIFY(d->opInfo.isValid());
	QIpcPointerLocker locker{d->opInfo};
	d->opInfo->mode = QtService::Service::OperationCompleted;
	locker.unlock();

	init();
	control = ServiceControl::create(backend(), name(), this);
	QVERIFY(control);
	QVERIFY2(control->serviceExists(), qUtf8Printable(control->error()));
	control->setBlocking(true);
}

void BasicServiceTest::cleanupTestCase()
{
	if(control)
		control->stop();
	cleanup();
}

void BasicServiceTest::testNameDetection()
{
	auto nameControl = ServiceControl::createFromName(backend(), QStringLiteral("testservice"), QStringLiteral("de.skycoder42.qtservice.tests"), this);
	QVERIFY(nameControl);
	QVERIFY(nameControl->serviceExists());
	QCOMPARE(nameControl->serviceId(), control->serviceId());
}

void BasicServiceTest::testStart()
{
	TEST_STATUS(ServiceControl::ServiceStopped);

	testFeature(ServiceControl::SupportsStart);
	QVERIFY2(control->start(), qUtf8Printable(control->error()));
	// blocking should only return after the server started, but for non blocking this may not be the case...
	if(!control->supportFlags().testFlag(ServiceControl::SupportsBlocking))
		QThread::sleep(3);

	socket = new QLocalSocket(this);
	socket->connectToServer(QStringLiteral("__qtservice_testservice"));
	QVERIFY(socket->waitForConnected(30000));
	stream.setDevice(socket);

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("started"));

	TEST_STATUS(ServiceControl::ServiceRunning);
}

void BasicServiceTest::testReload()
{
	TEST_STATUS(ServiceControl::ServiceRunning);

	testFeature(ServiceControl::SupportsReload);
	QVERIFY2(control->reload(), qUtf8Printable(control->error()));

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("reloading"));

	TEST_STATUS(ServiceControl::ServiceRunning);
}

void BasicServiceTest::testPause()
{
	TEST_STATUS(ServiceControl::ServiceRunning);

	testFeature(ServiceControl::SupportsPause);
	QVERIFY2(control->pause(), qUtf8Printable(control->error()));

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("pausing"));

	TEST_STATUS(ServiceControl::ServicePaused);
}

void BasicServiceTest::testResume()
{
	testFeature(ServiceControl::SupportsResume);
	TEST_STATUS(ServiceControl::ServicePaused);

	QVERIFY2(control->resume(), qUtf8Printable(control->error()));

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("resuming"));

	TEST_STATUS(ServiceControl::ServiceRunning);
}

void BasicServiceTest::testCustom()
{
	testFeature(ServiceControl::SupportsCustomCommands);
	testCustomImpl();
}

void BasicServiceTest::testStop()
{
	TEST_STATUS(ServiceControl::ServiceRunning);

	testFeature(ServiceControl::SupportsStop);
	QVERIFY2(control->stop(), qUtf8Printable(control->error()));

#ifndef Q_OS_WIN
	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("stopping"));
#endif
	QVERIFY(socket->waitForDisconnected(5000));

	TEST_STATUS(ServiceControl::ServiceStopped);
}

void BasicServiceTest::testAutostart()
{
	QVERIFY2(!control->isAutostartEnabled(), qUtf8Printable(control->error()));

	testFeature(ServiceControl::SupportsSetAutostart);
	QVERIFY2(control->enableAutostart(), qUtf8Printable(control->error()));
	QVERIFY2(control->isAutostartEnabled(), qUtf8Printable(control->error()));
	QVERIFY2(control->disableAutostart(), qUtf8Printable(control->error()));
	QVERIFY2(!control->isAutostartEnabled(), qUtf8Printable(control->error()));
}

QString BasicServiceTest::name()
{
	return QStringLiteral("testservice");
}

void BasicServiceTest::init() {}

void BasicServiceTest::cleanup() {}

void BasicServiceTest::testCustomImpl()
{
	QVERIFY2(false, "testCustomImpl not implemented");
}

void BasicServiceTest::performSocketTest()
{
	TEST_STATUS(ServiceControl::ServiceStopped);
	if(control->status() != ServiceControl::ServiceStopped)
		QThread::sleep(3); //leave some time for the svc to actually stop

	auto tcpSocket = new QTcpSocket(this);
	tcpSocket->connectToHost(QStringLiteral("127.0.0.1"), 15843);
	while(control->status() == ServiceControl::ServiceStarting)
		QThread::msleep(500);
	QVERIFY(tcpSocket->waitForConnected(5000));

	TEST_STATUS(ServiceControl::ServiceRunning);

	QByteArray msg = "hello world";
	tcpSocket->write(msg);

	QByteArray resMsg;
	do {
		QVERIFY(tcpSocket->waitForReadyRead(5000));
		resMsg += tcpSocket->readAll();
	} while(resMsg.size() < msg.size());
	QCOMPARE(resMsg, msg);

	testFeature(ServiceControl::SupportsStop);
	QVERIFY2(control->stop(), qUtf8Printable(control->error()));

	TEST_STATUS(ServiceControl::ServiceStopped);
}

void BasicServiceTest::testFeature(ServiceControl::SupportFlag flag)
{
	if(!control->supportFlags().testFlag(flag)) {
		auto meta = QMetaEnum::fromType<ServiceControl::SupportFlags>();
		if(flag == ServiceControl::SupportsStatus) {
			QEXPECT_FAIL("",
						 QByteArray(QByteArrayLiteral("feature ") + meta.valueToKey(flag) + QByteArrayLiteral(" not supported by backend")).constData(),
						 Continue);
		} else {
			QEXPECT_FAIL("",
						 QByteArray(QByteArrayLiteral("feature ") + meta.valueToKey(flag) + QByteArrayLiteral(" not supported by backend")).constData(),
						 Abort);
		}
	}
}

void BasicServiceTest::waitAsLongAs(ServiceControl::ServiceStatus status)
{
	static const QHash<ServiceControl::ServiceStatus, QList<ServiceControl::ServiceStatus>> statusMap {
		{ServiceControl::ServiceStopped, {ServiceControl::ServiceStopping}},
		{ServiceControl::ServiceRunning, {ServiceControl::ServiceStarting, ServiceControl::ServiceResuming, ServiceControl::ServiceReloading}},
		{ServiceControl::ServicePaused, {ServiceControl::ServicePausing}}
	};
	for(auto i = 0; i < 20 && statusMap[status].contains(control->status()); i++)
		QThread::msleep(500);
}
