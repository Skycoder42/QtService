#ifndef DUMMYSERVICEBACKEND_H
#define DUMMYSERVICEBACKEND_H

#include <QtCore/QPointer>
#include <QtService/ServiceBackend>

class DummyServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	explicit DummyServiceBackend(QObject *parent = nullptr);

	int runService(QtService::Service *service, int &argc, char **argv, int flags) override;

protected Q_SLOTS:
	void signalTriggered(int signal) override;

private:
	QPointer<QtService::Service> _service;
};

#endif // DUMMYSERVICEBACKEND_H
