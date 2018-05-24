#ifndef STANDARDSERVICEBACKEND_H
#define STANDARDSERVICEBACKEND_H

#include <QtCore/QPointer>
#include <QtService/ServiceBackend>

class StandardServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	explicit StandardServiceBackend(QObject *parent = nullptr);

	int runService(QtService::Service *service, int &argc, char **argv, int flags) override;
	void quitService() override;

protected Q_SLOTS:
	void signalTriggered(int signal) override;

private:
	QPointer<QtService::Service> _service;
};

#endif // STANDARDSERVICEBACKEND_H
