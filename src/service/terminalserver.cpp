#include "terminalserver_p.h"
#include "terminal_p.h"
using namespace QtService;

TerminalServer::TerminalServer(Service *service) :
	QObject{service},
	_service{service},
	_server{new QLocalServer{this}}
{}

bool TerminalServer::start(bool globally)
{
	_server->setSocketOptions(globally ? QLocalServer::WorldAccessOption : QLocalServer::UserAccessOption);
#ifdef Q_OS_WIN
	auto name = QStringLiteral("\\.\pipe\de\skycoder42\QtService\%1\terminal")
				.arg(QCoreApplication::applicationName());
#else
	auto name = _service->runtimeDir().absoluteFilePath(QStringLiteral("terminal.socket"));
#endif
	return _server->listen(name); //TODO log errors and retry!
}

void TerminalServer::stop()
{
	_server->close();
}

void TerminalServer::newConnection()
{
	while(_server->hasPendingConnections()) {
		auto terminal = new TerminalPrivate {
			_server->nextPendingConnection(),
			this
		};
		//TODO more stuff
	}
}
