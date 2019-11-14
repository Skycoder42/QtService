#include <QString>
#include <QtTest/QtTest>
#include <QtService/ServiceControl>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QProcess>
#include <unistd.h>
#include <basicservicetest.h>
using namespace QtService;

#define systemdpath QStringLiteral("systemd/user")
#define testservice QStringLiteral("testservice.service")
#define testsocket QStringLiteral("testservice.socket")

class TestSystemdService : public BasicServiceTest
{
	Q_OBJECT

protected:
	QString backend() override;
	QString name() override;
	void init() override;
	void cleanup() override;
	void testCustomImpl() override;
	bool resetFailed() override;

private Q_SLOTS:
	void testSocketActivation();
	void testReloadFail();

private:
	bool daemonReload();
};

QString TestSystemdService::backend()
{
	return QStringLiteral("systemd");
}

QString TestSystemdService::name()
{
	return testservice;
}

void TestSystemdService::init()
{
	QDir srcDir(QStringLiteral(SRCDIR));
	QVERIFY(srcDir.exists());
	QVERIFY(srcDir.exists(testservice));
	QVERIFY(srcDir.exists(testsocket));

	auto systemdHome = QDir{QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)};
	QVERIFY(systemdHome.mkpath(systemdpath));
	QVERIFY(systemdHome.cd(systemdpath));

	if(!systemdHome.exists(testservice)) {
		QFile in{srcDir.absoluteFilePath(testservice)};
		QVERIFY(in.open(QIODevice::ReadOnly | QIODevice::Text));
		QFile out{systemdHome.absoluteFilePath(testservice)};
		QVERIFY(out.open(QIODevice::WriteOnly | QIODevice::Text));
		out.write(in.readAll()
				  .replace("%{LD_LIBRARY_PATH}", qgetenv("LD_LIBRARY_PATH"))
				  .replace("%{QT_PLUGIN_PATH}", qgetenv("QT_PLUGIN_PATH"))
				  .replace("%{TESTSERVICE_PATH}", QString(QCoreApplication::applicationDirPath() + QStringLiteral("/../TestService/testservice")).toUtf8()));
	}
	if(!systemdHome.exists(testsocket))
		QVERIFY(QFile::copy(srcDir.absoluteFilePath(testsocket), systemdHome.absoluteFilePath(testsocket)));

	QVERIFY(daemonReload());
	resetFailed();
}

void TestSystemdService::cleanup()
{
	QProcess::execute(QStringLiteral("systemctl --user status testservice.service"));
	QProcess::execute(QStringLiteral("journalctl --user -xe --no-pager"));

	resetFailed();
	control->disableAutostart();
	auto systemdHome = QDir{QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)};
	QVERIFY(systemdHome.cd(systemdpath));
	QVERIFY(systemdHome.remove(testservice));
	QVERIFY(systemdHome.remove(testsocket));
	QVERIFY(daemonReload());
}

void TestSystemdService::testCustomImpl()
{
	QCOMPARE(control->status(), ServiceControl::Status::Running);

	// test pause
	QCOMPARE(control->callCommand<int>("kill", QStringLiteral("--signal=SIGTSTP")), EXIT_SUCCESS);
	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("pausing"));

	// test resume
	QCOMPARE(control->callCommand<int>("kill", QStringLiteral("--signal=SIGCONT")), EXIT_SUCCESS);
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("resuming"));
}

void TestSystemdService::testSocketActivation()
{
	resetFailed();

	auto socketControl = ServiceControl::create(backend(), QStringLiteral("testservice.socket"), this);
	QVERIFY(socketControl->start());

	performSocketTest();

	QVERIFY(socketControl->stop());
}

void TestSystemdService::testReloadFail()
{
	QVERIFY(control->setBlocking(true));
	QVERIFY2(control->start(), qUtf8Printable(control->error()));

	TEST_STATUS(ServiceControl::Status::Running);
	resetSettings({{QStringLiteral("fail"), true}});

	QVERIFY(!control->reload());

	TEST_STATUS(ServiceControl::Status::Errored);
	resetSettings();
	resetFailed();
	TEST_STATUS(ServiceControl::Status::Stopped);
}

bool TestSystemdService::daemonReload()
{
	QStringList args {QStringLiteral("daemon-reload")};
	if(::geteuid() != 0)
		args.prepend(QStringLiteral("--user"));
	return QProcess::execute(QStringLiteral("systemctl"), args) == EXIT_SUCCESS;
}

bool TestSystemdService::resetFailed()
{
	if(!control)
		return false;
	return control->callCommand<int>("reset-failed") == EXIT_SUCCESS;
}

QTEST_MAIN(TestSystemdService)

#include "tst_systemdservice.moc"
