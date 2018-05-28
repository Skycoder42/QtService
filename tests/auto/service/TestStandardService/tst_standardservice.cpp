#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <basicservicetest.h>

class TestStandardService : public BasicServiceTest
{
	Q_OBJECT

protected:
	QString backend() override;
	QString name() override;
	void init() override;
	void cleanup() override;

private Q_SLOTS:
	void testCreate();
};

QString TestStandardService::backend()
{
	return QStringLiteral("standard");
}

QString TestStandardService::name()
{
#ifdef Q_OS_WIN
#ifdef QT_NO_DEBUG
	return QStringLiteral(OUTDIR) + QStringLiteral("/../TestService/release/testservice");
#else
	return QStringLiteral(OUTDIR) + QStringLiteral("/../TestService/debug/testservice");
#endif
#else
	return QStringLiteral(OUTDIR) + QStringLiteral("/../TestService/testservice");
#endif
}

void TestStandardService::init()
{

}

void TestStandardService::cleanup()
{

}

void TestStandardService::testCreate()
{
	QVERIFY2(true, "Failure");
}

QTEST_MAIN(TestStandardService)

#include "tst_standardservice.moc"
