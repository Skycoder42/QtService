#include "basicservicetest.h"
#include <QtTest/QtTest>
using namespace QtService;

BasicServiceTest::BasicServiceTest(QObject *parent) :
	QObject(parent)
{}

void BasicServiceTest::initTestCase()
{
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
	testFeature(ServiceControl::SupportFlag::Start);
	QVERIFY2(control->start(), qUtf8Printable(control->error()));
	// blocking should only return after the server started, but for non blocking this may not be the case...
	if(control->blocking() != ServiceControl::BlockMode::Blocking)
		QThread::sleep(3);

	socket = new QLocalSocket(this);
	socket->connectToServer(QStringLiteral("__qtservice_testservice"));
	QVERIFY(socket->waitForConnected(30000));
	stream.setDevice(socket);

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("started"));

	TEST_STATUS(ServiceControl::Status::Running);
}

void BasicServiceTest::testReload()
{
	TEST_STATUS(ServiceControl::Status::Running);

	testFeature(ServiceControl::SupportFlag::Reload);
	QVERIFY2(control->reload(), qUtf8Printable(control->error()));

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("reloading"));

	TEST_STATUS(ServiceControl::Status::Running);
}

void BasicServiceTest::testPause()
{
	TEST_STATUS(ServiceControl::Status::Running);

	testFeature(ServiceControl::SupportFlag::Pause);
	QVERIFY2(control->pause(), qUtf8Printable(control->error()));

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("pausing"));

	TEST_STATUS(ServiceControl::Status::Paused);
}

void BasicServiceTest::testResume()
{
	testFeature(ServiceControl::SupportFlag::Resume);
	TEST_STATUS(ServiceControl::Status::Paused);

	QVERIFY2(control->resume(), qUtf8Printable(control->error()));

	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("resuming"));

	TEST_STATUS(ServiceControl::Status::Running);
}

void BasicServiceTest::testRestart()
{
	TEST_STATUS(ServiceControl::Status::Running);
	resetSettings();
	if(control->blocking() != ServiceControl::BlockMode::Blocking)
		return;
	testFeature(static_cast<ServiceControl::SupportFlag>(static_cast<int>(ServiceControl::SupportFlag::Start |
																		  ServiceControl::SupportFlag::Stop)));

	QVERIFY2(control->restart(), qUtf8Printable(control->error()));
	QThread::sleep(10);
	// blocking should only return after the server started, but for non blocking this may not be the case...
	if(control->blocking() != ServiceControl::BlockMode::Blocking)
		QThread::sleep(3);

	QByteArray msg;
#ifndef Q_OS_WIN
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("stopping"));
	msg = {};
#endif
	QVERIFY(socket->waitForDisconnected(5000));
	socket->deleteLater();

	socket = new QLocalSocket(this);
	socket->connectToServer(QStringLiteral("__qtservice_testservice"));
	QVERIFY(socket->waitForConnected(30000));
	stream.setDevice(socket);

	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("started"));

	TEST_STATUS(ServiceControl::Status::Running);
}

void BasicServiceTest::testCustom()
{
	testFeature(ServiceControl::SupportFlag::CustomCommands);
	testCustomImpl();
	resetFailed();
}

void BasicServiceTest::testStop()
{
	TEST_STATUS(ServiceControl::Status::Running);

	testFeature(ServiceControl::SupportFlag::Stop);
	QVERIFY2(control->stop(), qUtf8Printable(control->error()));

#ifndef Q_OS_WIN
	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("stopping"));
#endif
	QVERIFY(socket->waitForDisconnected(5000));

	TEST_STATUS(ServiceControl::Status::Stopped);
}

#ifndef Q_OS_WIN
void BasicServiceTest::testStartExit()
{
	resetSettings({{QStringLiteral("exit"), true}});

	TEST_STATUS(ServiceControl::Status::Stopped);

	testFeature(ServiceControl::SupportFlag::Start);
	if(control->start()) {
		if(control->blocking() != ServiceControl::BlockMode::Blocking)
			QThread::sleep(3);
	}

	TEST_STATUS(ServiceControl::Status::Stopped);
	resetSettings();
}

void BasicServiceTest::testStartFail()
{
	resetSettings({{QStringLiteral("fail"), true}});

	TEST_STATUS(ServiceControl::Status::Stopped);

	testFeature(ServiceControl::SupportFlag::Start);
	if(control->start()) {
		if(control->blocking() != ServiceControl::BlockMode::Blocking)
			QThread::sleep(3);
	}

	if(reportsStartErrors())
		TEST_STATUS(ServiceControl::Status::Errored);
	else
		TEST_STATUS(ServiceControl::Status::Stopped);

	QVERIFY(resetFailed());
	resetSettings();
}
#endif

void BasicServiceTest::testAutostart()
{
	QVERIFY2(!control->isAutostartEnabled(), qUtf8Printable(control->error()));

	testFeature(ServiceControl::SupportFlag::SetAutostart);
	QVERIFY2(control->enableAutostart(), qUtf8Printable(control->error()));
	QVERIFY2(control->isAutostartEnabled(), qUtf8Printable(control->error()));
	QVERIFY2(control->disableAutostart(), qUtf8Printable(control->error()));
	QVERIFY2(!control->isAutostartEnabled(), qUtf8Printable(control->error()));
}

void BasicServiceTest::testDisable()
{
	QVERIFY2(control->isEnabled(), qUtf8Printable(control->error()));

	testFeature(ServiceControl::SupportFlag::SetEnabled);
	QVERIFY2(control->setEnabled(false), qUtf8Printable(control->error()));
	QVERIFY2(!control->isEnabled(), qUtf8Printable(control->error()));

	if (control->start()) {
		// start command succeded - wait for the service to not reach running status
		for(auto i = 0; i < 5; ++i) {
			QThread::msleep(1000);
			TEST_STATUS(ServiceControl::Status::Stopped);
		}
	}

	QVERIFY2(control->setEnabled(true), qUtf8Printable(control->error()));
	QVERIFY2(control->isEnabled(), qUtf8Printable(control->error()));
}

QString BasicServiceTest::name()
{
	return QStringLiteral("testservice");
}

bool BasicServiceTest::reportsStartErrors()
{
	return true;
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
	QSettings config{control->runtimeDir().absoluteFilePath(QStringLiteral("test.conf")), QSettings::IniFormat};
	QVERIFY(config.isWritable());
	config.clear();
	for(auto it = args.constBegin(); it != args.constEnd(); ++it)
		config.setValue(it.key(), it.value());
	config.sync();
}

void BasicServiceTest::performSocketTest()
{
	TEST_STATUS(ServiceControl::Status::Stopped);
	if(control->status() != ServiceControl::Status::Stopped)
		QThread::sleep(3); //leave some time for the svc to actually stop

	auto tcpSocket = new QTcpSocket(this);
	tcpSocket->connectToHost(QStringLiteral("127.0.0.1"), 15843);
	while(control->status() == ServiceControl::Status::Starting)
		QThread::msleep(500);
	QVERIFY(tcpSocket->waitForConnected(5000));

	TEST_STATUS(ServiceControl::Status::Running);

	QByteArray msg = "hello world";
	tcpSocket->write(msg);

	QByteArray resMsg;
	do {
		QVERIFY(tcpSocket->waitForReadyRead(5000));
		resMsg += tcpSocket->readAll();
	} while(resMsg.size() < msg.size());
	QCOMPARE(resMsg, msg);

	testFeature(ServiceControl::SupportFlag::Stop);
	QVERIFY2(control->stop(), qUtf8Printable(control->error()));

	TEST_STATUS(ServiceControl::Status::Stopped);
}

void BasicServiceTest::testFeature(ServiceControl::SupportFlag flag)
{
	if(!control->supportFlags().testFlag(flag)) {
		auto meta = QMetaEnum::fromType<ServiceControl::SupportFlags>();
		if(flag == ServiceControl::SupportFlag::Status) {
			QEXPECT_FAIL("",
						 QByteArray(QByteArrayLiteral("feature ") + meta.valueToKey(static_cast<int>(flag)) + QByteArrayLiteral(" not supported by backend")).constData(),
						 Continue);
		} else {
			QEXPECT_FAIL("",
						 QByteArray(QByteArrayLiteral("feature ") + meta.valueToKey(static_cast<int>(flag)) + QByteArrayLiteral(" not supported by backend")).constData(),
						 Abort);
		}
	}
}

void BasicServiceTest::waitAsLongAs(ServiceControl::Status status)
{
	static const QHash<ServiceControl::Status, QList<ServiceControl::Status>> statusMap {
		{ServiceControl::Status::Stopped, {ServiceControl::Status::Stopping, ServiceControl::Status::Starting}},
		{ServiceControl::Status::Running, {ServiceControl::Status::Starting, ServiceControl::Status::Resuming, ServiceControl::Status::Reloading}},
		{ServiceControl::Status::Paused, {ServiceControl::Status::Pausing}},
		{ServiceControl::Status::Errored, {ServiceControl::Status::Stopping, ServiceControl::Status::Starting}}
	};
	for(auto i = 0; i < 20 && statusMap[status].contains(control->status()); i++)
		QThread::msleep(500);
}
