#ifndef SYSTEMDSERVICEBACKEND_H
#define SYSTEMDSERVICEBACKEND_H

#include <QtCore/QTimer>
#include <QtCore/QLoggingCategory>

#include <QtDBus/QDBusConnection>

#include <QtService/ServiceBackend>

#include "systemd_adaptor.h"

class SystemdServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	static const QString DBusObjectPath;

	explicit SystemdServiceBackend(QtService::Service *service);

	int runService(int &argc, char **argv, int flags) override;
	Q_INVOKABLE void quitService() override;
	Q_INVOKABLE void reloadService() override;
	QList<int> getActivatedSockets(const QByteArray &name) override;

protected Q_SLOTS:
	void signalTriggered(int signal) override;

private Q_SLOTS:
	void sendWatchdog();

	void onStarted(bool success);
	void onReloaded(bool success);
	void onStopped(int exitCode);
	void onPaused(bool success);

private:
	bool _userService = true;
	QTimer *_watchdogTimer = nullptr;
	QMultiHash<QByteArray, int> _sockets;

	SystemdAdaptor *_dbusAdapter;

	int run();
	int stop();
	int reload();

	void prepareWatchdog();

	QDBusConnection dbusConnection() const;
	QString dbusId() const;
	void printDbusError(const QDBusError &error) const;

	static void systemdMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
};

Q_DECLARE_LOGGING_CATEGORY(logBackend)

#endif // SYSTEMDSERVICEBACKEND_H
