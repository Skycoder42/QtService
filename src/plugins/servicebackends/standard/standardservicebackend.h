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
	void reloadService() override;

protected Q_SLOTS:
	void signalTriggered(int signal) override;

private:
	QPointer<QtService::Service> _service; //TODO move to baseclass with "pre-start-setter" + simplify methods
};

#endif // STANDARDSERVICEBACKEND_H
