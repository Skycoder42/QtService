#ifndef STANDARDSERVICEBACKEND_H
#define STANDARDSERVICEBACKEND_H

#include <QtCore/QPointer>
#include <QtCore/QLoggingCategory>

#include <QtService/ServiceBackend>

class StandardServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	explicit StandardServiceBackend(bool debugMode, QtService::Service *service);

	int runService(int &argc, char **argv, int flags) override;
	void quitService() override;
	void reloadService() override;

protected Q_SLOTS:
	void signalTriggered(int signal) override;

private Q_SLOTS:
	void onStarted(bool success);
	void onPaused(bool success);

private:
	const bool _debugMode;
};

Q_DECLARE_LOGGING_CATEGORY(logBackend)

#endif // STANDARDSERVICEBACKEND_H
