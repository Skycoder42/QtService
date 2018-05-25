#ifndef SYSTEMDSERVICEBACKEND_H
#define SYSTEMDSERVICEBACKEND_H

#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtNetwork/QLocalServer>
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
	void performStart();
	void sendWatchdog();

	void newConnection();

	void onReady();
	void onStopped(int exitCode);

private:
	QPointer<QtService::Service> _service;
	QTimer *_watchdogTimer = nullptr;
	QLocalServer *_commandServer = nullptr;

	int run();
	int stop();
	int reload();

	void prepareWatchdog();
	QString getSocketName();

	static void systemdMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
};

#endif // SYSTEMDSERVICEBACKEND_H
