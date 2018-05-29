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
	void testStop();

protected:
	QtService::ServiceControl *control = nullptr;
	QLocalSocket *socket = nullptr;
	QDataStream stream;

	virtual QString backend() = 0;
	virtual QString name();
	virtual void init();
	virtual void cleanup();

	void testFeature(QtService::ServiceControl::SupportFlag flag);
};

#define READ_LOOP(...) do { \
	QVERIFY(socket->waitForReadyRead(5000)); \
	stream.startTransaction(); \
	stream >> __VA_ARGS__; \
} while(!stream.commitTransaction())

#endif // BASICSERVICETEST_H
