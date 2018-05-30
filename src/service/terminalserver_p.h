#ifndef QTSERVICE_TERMINALSERVER_P_H
#define QTSERVICE_TERMINALSERVER_P_H

#include <QtCore/QObject>
#include <QtNetwork/QLocalServer>

#include "terminal.h"
#include "service.h"

namespace QtService {

class TerminalServer : public QObject
{
	Q_OBJECT

public:
	explicit TerminalServer(Service *service);

	bool start(bool globally);
	void stop();

Q_SIGNALS:
	void terminalConnected(QtService::Terminal *terminal);

private Q_SLOTS:
	void newConnection();

private:
	Service *_service;
	QLocalServer *_server;
};

}

#endif // QTSERVICE_TERMINALSERVER_P_H
