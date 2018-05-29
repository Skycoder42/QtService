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

private Q_SLOTS:
	void testSocketActivation();

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
}

void TestSystemdService::cleanup()
{
	auto systemdHome = QDir{QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)};
	QVERIFY(systemdHome.cd(systemdpath));
	QVERIFY(systemdHome.remove(testservice));
	QVERIFY(systemdHome.remove(testsocket));
	QVERIFY(daemonReload());
}

void TestSystemdService::testCustomImpl()
{
	QCOMPARE(control->status(), ServiceControl::ServiceRunning);

	QCOMPARE(control->callCommand<int>("kill", QStringLiteral("--signal=SIGTSTP")), EXIT_SUCCESS);
	QByteArray msg;
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("pausing"));

	QCOMPARE(control->callCommand<int>("kill", QStringLiteral("--signal=SIGCONT")), EXIT_SUCCESS);
	READ_LOOP(msg);
	QCOMPARE(msg, QByteArray("resuming"));
}

void TestSystemdService::testSocketActivation()
{
	auto socketControl = ServiceControl::create(backend(), QStringLiteral("testservice.socket"), this);
	QVERIFY(socketControl->start());

	performSocketTest();

	QVERIFY(socketControl->stop());
}

bool TestSystemdService::daemonReload()
{
	QStringList args {QStringLiteral("daemon-reload")};
	if(::geteuid() != 0)
		args.prepend(QStringLiteral("--user"));
	return QProcess::execute(QStringLiteral("systemctl"), args) == EXIT_SUCCESS;
}

QTEST_MAIN(TestSystemdService)

#include "tst_systemdservice.moc"
