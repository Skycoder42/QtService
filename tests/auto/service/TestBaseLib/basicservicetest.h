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

	void testStart();
	void testReload();
	void testPause();
	void testResume();
	void testCustom();
	void testStop();
	void testAutostart();

protected:
	QtService::ServiceControl *control = nullptr;
	QLocalSocket *socket = nullptr;
	QDataStream stream;

	virtual QString backend() = 0;
	virtual QString name();
	virtual void init();
	virtual void cleanup();

	virtual void testCustomImpl();

	void performSocketTest();
	void testFeature(QtService::ServiceControl::SupportFlag flag);
	bool waitAsLongAs(QtService::ServiceControl::ServiceStatus status);
};

#define READ_LOOP(...) do { \
	QVERIFY(socket->waitForReadyRead(30000)); \
	stream.startTransaction(); \
	stream >> __VA_ARGS__; \
} while(!stream.commitTransaction())

#define TEST_STATUS(state) \
	if(control->supportFlags().testFlag(ServiceControl::SupportsStatus)) \
		QCOMPARE(control->status(), state)

#endif // BASICSERVICETEST_H
