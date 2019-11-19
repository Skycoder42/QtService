#ifndef QTSERVICE_TERMINALSERVER_P_H
#define QTSERVICE_TERMINALSERVER_P_H

#include "terminal.h"
#include "service.h"

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>

#include <QtNetwork/QLocalServer>

namespace QtService {

class TerminalPrivate;
class TerminalServer : public QObject
{
	Q_OBJECT

public:
	explicit TerminalServer(Service *service);

	static QString serverName();

	bool start(bool globally);
	void stop();

	bool isRunning() const;

Q_SIGNALS:
	void terminalConnected(QtService::Terminal *terminal);

private Q_SLOTS:
	void newConnection();

	void terminalReady(TerminalPrivate *terminal, bool success);

private:
	Service *_service;
	QLocalServer *_server;
	bool _activated = false;

	bool setSocketDescriptor(int socket);
};

Q_DECLARE_LOGGING_CATEGORY(logTermServer)

}

#endif // QTSERVICE_TERMINALSERVER_P_H
