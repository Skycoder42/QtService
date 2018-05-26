#ifndef SYSTEMDSERVICEBACKEND_H
#define SYSTEMDSERVICEBACKEND_H

#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtService/ServiceBackend>

class SystemdServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	explicit SystemdServiceBackend(QObject *parent = nullptr);

	int runService(QtService::Service *service, int &argc, char **argv, int flags) override;
	void quitService() override;
	void reloadService() override;
	QHash<int, QByteArray> getActivatedSockets() override;

protected Q_SLOTS:
	void signalTriggered(int signal) override;

private Q_SLOTS:
	void sendWatchdog();

	void onReady();
	void onStopped(int exitCode);

private:
	QPointer<QtService::Service> _service;
	QTimer *_watchdogTimer = nullptr;

	int run(int &argc, char **argv, int flags);
	int stop(int pid);
	int reload(int pid);

	void prepareWatchdog();
	bool findArg(const char *command, int argc, char **argv, int &pid);

	static void systemdMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
};

#endif // SYSTEMDSERVICEBACKEND_H