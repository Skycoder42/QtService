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
	QList<int> getActivatedSockets(const QByteArray &name) override;

protected Q_SLOTS:
	void signalTriggered(int signal) override;

private Q_SLOTS:
	void onStarted(bool success);
	void onPaused(bool success);

private:
	QMultiHash<QByteArray, int> _socketCache;

	static void syslogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
};

#endif // LAUNCHDSERVICEBACKEND_H
