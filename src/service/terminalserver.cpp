#include "terminalserver_p.h"
#include "terminal_p.h"
#include "logging_p.h"
using namespace QtService;

TerminalServer::TerminalServer(Service *service) :
	QObject{service},
	_service{service},
	_server{new QLocalServer{this}}
{}

bool TerminalServer::start(bool globally)
{
	_server->setSocketOptions(globally ? QLocalServer::WorldAccessOption : QLocalServer::UserAccessOption);
	auto activeSockets = _service->getSockets("terminal");
	if(activeSockets.isEmpty()) {
#ifdef Q_OS_WIN
		auto name = QStringLiteral("\\.\pipe\de.skycoder42.QtService.%1.terminal")
				.arg(QCoreApplication::applicationName());
#else
		auto name = _service->runtimeDir().absoluteFilePath(QStringLiteral("terminal.socket"));
#endif
		if(!_server->listen(name)) {
			if(_server->serverError() == QAbstractSocket::AddressInUseError) {
				if(QLocalServer::removeServer(name))
					_server->listen(name);
			}
		}
	} else {
		if(_activated)
			qCWarning(logQtService) << "Reopening an already closed activated socket is not supported and will result in undefined behaviour!";
		if(activeSockets.size() > 1)
			qCWarning(logQtService) << "Found more then 1 activated terminal socket - using first one:" << activeSockets.first();
		_activated = _server->listen(activeSockets.first()) || _activated;
	}

	if(_server->isListening())
		return true;
	else {
		qCCritical(logQtService) << "Failed to create terminal server with error:" << _server->errorString();
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
	while(_server->hasPendingConnections()) {
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
	if(success)
		emit terminalConnected(new Terminal{terminal, this});
	else
		terminal->deleteLater();
}
