#include "terminalserver_p.h"
#include "terminal_p.h"
#include "service_p.h"
using namespace QtService;

Q_LOGGING_CATEGORY(QtService::logTermServer, "qt.service.terminal.server")

TerminalServer::TerminalServer(Service *service) :
	QObject{service},
	_service{service},
	_server{new QLocalServer{this}}
{
	connect(_server, &QLocalServer::newConnection,
			this, &TerminalServer::newConnection);
}

QString TerminalServer::serverName()
{
#ifdef Q_OS_WIN
	return QStringLiteral(R"__(\\.\pipe\de.skycoder42.QtService.%1.terminal)__")
				.arg(QCoreApplication::applicationName());
#else
	return ServicePrivate::runtimeDir().absoluteFilePath(QStringLiteral("terminal.socket"));
#endif
}

bool TerminalServer::start(bool globally)
{
	_server->setSocketOptions(globally ? QLocalServer::WorldAccessOption : QLocalServer::UserAccessOption);
	auto activeSockets = _service->getSockets("terminal");
	if (activeSockets.isEmpty()) {
		auto name = serverName();
		if (!_server->listen(name)) {
			if (_server->serverError() == QAbstractSocket::AddressInUseError) {
				if (QLocalServer::removeServer(name))
					_server->listen(name);
			}
		}
	} else {
		if (_activated)
			qCWarning(logTermServer) << "Reopening an already closed activated socket is not supported and will result in undefined behaviour!";
		if (activeSockets.size() > 1)
			qCWarning(logTermServer) << "Found more then 1 activated terminal socket - using first one:" << activeSockets.first();
		_activated = _server->listen(activeSockets.first()) || _activated;
	}

	if (_server->isListening())
		return true;
	else {
		qCCritical(logTermServer) << "Failed to create terminal server with error:" << _server->errorString();
		return false;
	}
}

void TerminalServer::stop()
{
	_server->close();
}

bool TerminalServer::isRunning() const
{
	return _server->isListening();
}

void TerminalServer::newConnection()
{
	while (_server->hasPendingConnections()) {
		auto terminal = new TerminalPrivate {
			_server->nextPendingConnection(),
			this
		};
		connect(terminal, &TerminalPrivate::terminalReady,
				this, &TerminalServer::terminalReady);
	}
}

void TerminalServer::terminalReady(TerminalPrivate *terminal, bool success)
{
	if (success)
		emit terminalConnected(new Terminal{terminal, _service});
	else
		terminal->deleteLater();
}
