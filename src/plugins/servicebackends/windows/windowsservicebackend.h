#ifndef WINDOWSSERVICEBACKEND_H
#define WINDOWSSERVICEBACKEND_H

#include <QtService/ServiceBackend>

class WindowsServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	explicit WindowsServiceBackend(QObject *parent = nullptr);

	int runService(QtService::Service *service, int &argc, char **argv, int flags) override;
	void quitService() override;
	void reloadService() override;

private:
	QtService::Service *_service;
};

#endif // WINDOWSSERVICEBACKEND_H
