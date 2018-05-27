#ifndef LAUNCHDSERVICEBACKEND_H
#define LAUNCHDSERVICEBACKEND_H

#include <QtService/ServiceBackend>

class LaunchdServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	explicit LaunchdServiceBackend(QtService::Service *service);

public:
	int runService(int &argc, char **argv, int flags) override;
	void quitService() override;
	void reloadService() override;
	QHash<int, QByteArray> getActivatedSockets() override;

protected Q_SLOTS:
	void signalTriggered(int signal) override;

private Q_SLOTS:
	void onPaused();
};

#endif // LAUNCHDSERVICEBACKEND_H
