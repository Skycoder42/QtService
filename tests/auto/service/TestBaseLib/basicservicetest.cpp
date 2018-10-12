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
	QVERIFY2(control->serviceExists(), qUtf8Printable(control->error()));
	control->setBlocking(true);

	resetSettings();
}

void BasicServiceTest::cleanupTestCase()
{
	if(control) {
		control->stop();
		resetSettings();
	}
	cleanup();
}

void BasicServiceTest::testBasics()
{
	QCOMPARE(control->backend(), backend());
	QCOMPARE(control->serviceId(), name());
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

void BasicServiceTest::testStartExit()
{
	resetSettings({{QStringLiteral("exit"), true}});

	TEST_STATUS(ServiceControl::ServiceStopped);

	testFeature(ServiceControl::SupportsStart);
	if(control->start()) {
		if(!control->supportFlags().testFlag(ServiceControl::SupportsBlocking))
			QThread::sleep(3);
	}

	TEST_STATUS(ServiceControl::ServiceStopped);
	resetSettings();
}

void BasicServiceTest::testStartFail()
{
	resetSettings({{QStringLiteral("fail"), true}});

	TEST_STATUS(ServiceControl::ServiceStopped);

	testFeature(ServiceControl::SupportsStart);
	if(control->start()) {
		if(!control->supportFlags().testFlag(ServiceControl::SupportsBlocking))
			QThread::sleep(3);
	}

	if(reportsStartErrors())
		TEST_STATUS(ServiceControl::ServiceErrored);
	else
		TEST_STATUS(ServiceControl::ServiceStopped);

	QVERIFY(resetFailed());
	resetSettings();
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

bool BasicServiceTest::reportsStartErrors()
{
	return true;
}

QDir BasicServiceTest::runtimeDir()
{
	return control->runtimeDir();
}

void BasicServiceTest::init() {}

void BasicServiceTest::cleanup() {}

bool BasicServiceTest::resetFailed()
{
	return true;
}

void BasicServiceTest::testCustomImpl()
{
	QVERIFY2(false, "testCustomImpl not implemented");
}

void BasicServiceTest::resetSettings(const QVariantHash &args)
{
	QSettings config{runtimeDir().absoluteFilePath(QStringLiteral("test.conf")), QSettings::IniFormat};
	QVERIFY(config.isWritable());
	config.clear();
	config.setValue(QStringLiteral("testval"), true);
	for(auto it = args.constBegin(); it != args.constEnd(); ++it)
		config.setValue(it.key(), it.value());
	config.sync();
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
		{ServiceControl::ServiceStopped, {ServiceControl::ServiceStopping, ServiceControl::ServiceStarting}},
		{ServiceControl::ServiceRunning, {ServiceControl::ServiceStarting, ServiceControl::ServiceResuming, ServiceControl::ServiceReloading}},
		{ServiceControl::ServicePaused, {ServiceControl::ServicePausing}},
		{ServiceControl::ServiceErrored, {ServiceControl::ServiceStopping, ServiceControl::ServiceStarting}}
	};
	for(auto i = 0; i < 20 && statusMap[status].contains(control->status()); i++)
		QThread::msleep(500);
}
