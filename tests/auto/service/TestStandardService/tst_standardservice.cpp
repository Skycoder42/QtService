#include <QString>
#include <QtTest/QtTest>
#include <QCoreApplication>
#include <basicservicetest.h>

class TestStandardService : public BasicServiceTest
{
	Q_OBJECT

protected:
	void init() override;
	QString backend() override;
	QString name() override;
};

void TestStandardService::init()
{
#ifdef Q_OS_WIN
#ifdef QT_NO_DEBUG
	QString cPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/release");
#else
	QString cPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/debug");
#endif
#else
	QString cPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../TestService");
#endif
	cPath = QDir::cleanPath(cPath);
	cPath += QDir::listSeparator() + qEnvironmentVariable("PATH");
	qputenv("PATH", cPath.toUtf8());
}

QString TestStandardService::backend()
{
#ifdef QT_NO_DEBUG
	return QStringLiteral("standard");
#else
	return QStringLiteral("debug");
#endif
}

QString TestStandardService::name()
{
#ifdef Q_OS_WIN
#ifdef QT_NO_DEBUG
	QString svcPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/release/testservice.exe");
#else
	QString svcPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/debug/testservice.exe");
#endif
#else
	QString svcPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../TestService/testservice");
#endif
	return QDir::cleanPath(svcPath);
}

QTEST_MAIN(TestStandardService)

#include "tst_standardservice.moc"
