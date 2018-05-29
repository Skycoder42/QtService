#include <QString>
#include <QtTest/QtTest>
#include <QCoreApplication>
#include <basicservicetest.h>

class TestStandardService : public BasicServiceTest
{
	Q_OBJECT

protected:
	QString backend() override;
	QString name() override;
};

QString TestStandardService::backend()
{
	return QStringLiteral("standard");
}

QString TestStandardService::name()
{
#ifdef Q_OS_WIN
#ifdef QT_NO_DEBUG
	return QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/release/testservice");
#else
	return QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/debug/testservice");
#endif
#else
	return QCoreApplication::applicationDirPath() + QStringLiteral("/../TestService/testservice");
#endif
}
QTEST_MAIN(TestStandardService)

#include "tst_standardservice.moc"
