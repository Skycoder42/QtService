#ifndef QTSERVICE_SERVICE_P_H
#define QTSERVICE_SERVICE_P_H

#include <QtCore/QPointer>

#include "service.h"
#include "servicebackend.h"
#include "terminalserver_p.h"

namespace QtService {

class ServiceControl;
class ServicePrivate
{
public:
	ServicePrivate(Service *q_ptr, int &argc, char **argv, int flags);

	static QStringList listBackends();
	static ServiceControl *createControl(const QString &provider, QString &&serviceId, QObject *parent);
	static ServiceControl *createLocalControl(const QString &provider, QObject *parent);
	static QDir runtimeDir(const QString &serviceName = QCoreApplication::applicationName());

	static QPointer<Service> instance;

	int &argc;
	char **argv;
	int flags;

	QString backendProvider;
	ServiceBackend *backend = nullptr;
	QHash<QByteArray, std::function<QVariant(QVariantList)>> callbacks;

	bool isRunning = false;
	bool wasPaused = false;
	bool terminalActive = false;
	Service::TerminalMode terminalMode = Service::ReadWriteActive;
	bool terminalGlobal = false;
	bool startWithTerminal = false;

	TerminalServer *termServer = nullptr;

	void startTerminals();
	void stopTerminals();

private:
	Service *q;
};

}

#endif // QTSERVICE_SERVICE_P_H
