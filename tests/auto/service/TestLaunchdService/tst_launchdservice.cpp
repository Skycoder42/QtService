#include <QString>
#include <QtTest/QtTest>
#include <QCoreApplication>
#include <basicservicetest.h>

#define launchdpath QStringLiteral("Library/LaunchAgents")
#define testservice QStringLiteral("de.skycoder42.qtservice.tests.testservice.plist")

class TestLaunchdService : public BasicServiceTest
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
	bool launchdLoad(const QString &svc, bool load);
};

QString TestLaunchdService::backend()
{
	return QStringLiteral("launchd");
}

QString TestLaunchdService::name()
{
	return QStringLiteral("de.skycoder42.qtservice.tests.testservice");
}

void TestLaunchdService::init()
{
	QDir srcDir(QStringLiteral(SRCDIR));
	QVERIFY(srcDir.exists());
	QVERIFY(srcDir.exists(testservice));

	auto launchdHome = QDir{QStandardPaths::writableLocation(QStandardPaths::HomeLocation)};
	QVERIFY(launchdHome.mkpath(launchdpath));
	QVERIFY(launchdHome.cd(launchdpath));

	if(!launchdHome.exists(testservice)) {
		QFile in{srcDir.absoluteFilePath(testservice)};
		QVERIFY(in.open(QIODevice::ReadOnly | QIODevice::Text));
		QFile out{launchdHome.absoluteFilePath(testservice)};
		QVERIFY(out.open(QIODevice::WriteOnly | QIODevice::Text));
		out.write(in.readAll()
				  .replace("%{DYLD_LIBRARY_PATH}", qgetenv("DYLD_LIBRARY_PATH"))
				  .replace("%{DYLD_FRAMEWORK_PATH}", qgetenv("DYLD_FRAMEWORK_PATH"))
				  .replace("%{QT_PLUGIN_PATH}", qgetenv("QT_PLUGIN_PATH"))
				  .replace("%{TESTSERVICE_PATH}", QString(QCoreApplication::applicationDirPath() + QStringLiteral("/../TestService/testservice")).toUtf8()));
	}

	QVERIFY(launchdLoad(launchdHome.absoluteFilePath(testservice), true));
}

void TestLaunchdService::cleanup()
{
	auto launchdHome = QDir{QStandardPaths::writableLocation(QStandardPaths::HomeLocation)};
	QVERIFY(launchdHome.cd(launchdpath));
	launchdLoad(launchdHome.absoluteFilePath(testservice), false);
	QVERIFY(launchdHome.remove(testservice));
}

void TestLaunchdService::testCustomImpl()
{
	QCOMPARE(control->callCommand<int>("list"), EXIT_SUCCESS);
}

void TestLaunchdService::testSocketActivation()
{
	performSocketTest();
}

bool TestLaunchdService::launchdLoad(const QString &svc, bool load)
{
	QStringList args {
		load ? QStringLiteral("load") : QStringLiteral("unload"),
		svc
	};
	return QProcess::execute(QStringLiteral("launchctl"), args) == EXIT_SUCCESS;
}

QTEST_MAIN(TestLaunchdService)

#include "tst_launchdservice.moc"
