#ifndef BASICSERVICETEST_H
#define BASICSERVICETEST_H

#include <QtService/Service>
#include <QtService/ServiceControl>
#include <QtNetwork/QLocalSocket>
#include <QtCore/QDataStream>

class BasicServiceTest : public QObject
{
	Q_OBJECT

public:
	explicit BasicServiceTest(QObject *parent = nullptr);

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testBasics();
	void testNameDetection();

	void testStart();
	void testReload();
	void testPause();
	void testResume();
	void testRestart();
	void testCustom();
	void testStop();

#ifndef Q_OS_WIN
	void testStartExit();
	void testStartFail();
#endif

	void testAutostart();
	void testDisable();

protected:
	QtService::ServiceControl *control = nullptr;
	QLocalSocket *socket = nullptr;
	QDataStream stream;

	virtual QString backend() = 0;
	virtual QString name();
	virtual bool reportsStartErrors();
	virtual void init();
	virtual void cleanup();
	virtual bool resetFailed();

	virtual void testCustomImpl();

	void resetSettings(const QVariantHash &args = {});
	void performSocketTest();
	void testFeature(QtService::ServiceControl::SupportFlag flag);
	void waitAsLongAs(QtService::ServiceControl::Status status);
};

#define READ_LOOP(...) do { \
	QVERIFY(socket->waitForReadyRead(30000)); \
	stream.startTransaction(); \
	stream >> __VA_ARGS__; \
} while(!stream.commitTransaction())

#define TEST_STATUS(state) do {\
	if(control->supportFlags().testFlag(ServiceControl::SupportFlag::Status)) { \
		waitAsLongAs(state); \
		QCOMPARE(control->status(), state); \
	} \
} while(false)

#endif // BASICSERVICETEST_H
