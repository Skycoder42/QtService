#ifndef BASICSERVICETEST_H
#define BASICSERVICETEST_H

#include <QtService/Service>
#include <QtService/ServiceControl>

class BasicServiceTest : public QObject
{
	Q_OBJECT

public:
	explicit BasicServiceTest(QObject *parent = nullptr);

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

protected:
	QtService::ServiceControl *control = nullptr;

	virtual QString backend() = 0;
	virtual QString name();
	virtual void init();
	virtual void cleanup();

	void testFeature(QtService::ServiceControl::SupportFlags flags);
};

#endif // BASICSERVICETEST_H
