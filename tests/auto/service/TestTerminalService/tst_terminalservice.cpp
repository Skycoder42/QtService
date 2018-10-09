#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QProcess>

class TestTerminalService : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testPassiveTerminal();
	void testActiveTerminal();
	void testTermStop();

private:
	QString svcPath;

	QProcess *createProc(QStringList args = {});
};

void TestTerminalService::initTestCase()
{
#ifdef Q_OS_WIN
#ifdef QT_NO_DEBUG
	svcPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/release/testservice.exe");
#else
	svcPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/debug/testservice.exe");
#endif
#else
	svcPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../TestService/testservice");
#endif
	QVERIFY2(QFile::exists(svcPath), qUtf8Printable(svcPath));
}

void TestTerminalService::cleanupTestCase()
{
}

void TestTerminalService::testPassiveTerminal()
{
	auto proc = createProc({QStringLiteral("--passive")});
	QVERIFY2(proc->waitForStarted(5000), qUtf8Printable(proc->errorString()));
	QThread::sleep(2);

	proc->write("terminal test\n");
	QVERIFY(proc->waitForBytesWritten(5000));
	QVERIFY(proc->waitForReadyRead(5000));
	QCOMPARE(proc->readAll(), QByteArray{"terminal test\n"});

	proc->write("another test\n");
	QVERIFY(proc->waitForBytesWritten(5000));
	QVERIFY(proc->waitForReadyRead(5000));
	QCOMPARE(proc->readAll(), QByteArray{"another test\n"});

	proc->terminate();
	if(!proc->waitForFinished(5000))
		proc->kill();

	proc->deleteLater();
}

void TestTerminalService::testActiveTerminal()
{
	auto proc = createProc();
	QVERIFY2(proc->waitForStarted(5000), qUtf8Printable(proc->errorString()));

	QVERIFY(proc->waitForReadyRead(5000));
	QCOMPARE(proc->readAll(), QByteArray{"name: "});

	proc->write("terminal test\n");
	QVERIFY(proc->waitForBytesWritten(5000));
	QVERIFY(proc->waitForFinished(5000));

	proc->deleteLater();
}

void TestTerminalService::testTermStop()
{
	auto proc = createProc({QStringLiteral("stop")});
	QVERIFY2(proc->waitForStarted(5000), qUtf8Printable(proc->errorString()));
	QVERIFY(proc->waitForFinished(5000));

	proc->deleteLater();
}

QProcess *TestTerminalService::createProc(QStringList args)
{
	args.prepend(QStringLiteral("--terminal"));
	args.prepend(QStringLiteral("standard"));
	args.prepend(QStringLiteral("--backend"));

	auto proc = new QProcess{this};
	proc->setProgram(svcPath);
	proc->setArguments(args);
	proc->setProcessChannelMode(QProcess::ForwardedErrorChannel);
	proc->start(QIODevice::ReadWrite | QIODevice::Text);

	return proc;
}

QTEST_MAIN(TestTerminalService)

#include "tst_terminalservice.moc"
