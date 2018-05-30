#ifndef QTSERVICE_TERMINALSERVER_P_H
#define QTSERVICE_TERMINALSERVER_P_H

#include <QtCore/QObject>
#include <QtNetwork/QLocalServer>

#include "terminal.h"
#include "service.h"

namespace QtService {

class TerminalPrivate;
class TerminalServer : public QObject
{
	Q_OBJECT

public:
	explicit TerminalServer(Service *service);

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

}

#endif // QTSERVICE_TERMINALSERVER_P_H
