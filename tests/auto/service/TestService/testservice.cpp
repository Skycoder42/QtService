#include "testservice.h"
#include <QtService/Terminal>

#include <QTimer>
#include <QTcpSocket>
using namespace QtService;

TestService::TestService(int &argc, char **argv) :
	Service{argc, argv}
{
	setTerminalActive(true);
	setStartWithTerminal(true);
}

bool TestService::preStart()
{
	qDebug() << Q_FUNC_INFO;
	return true;
}

Service::CommandMode TestService::onStart()
{
	qDebug() << Q_FUNC_INFO;

	_server = new QLocalServer(this);
	_server->setSocketOptions(QLocalServer::WorldAccessOption);
	connect(_server, &QLocalServer::newConnection, this, [this](){
		qDebug() << "new connection";
		if(_socket)
			return;
		_socket = _server->nextPendingConnection();
		_stream.setDevice(_socket);
		_server->close();

		emit started();
		_stream << QByteArray("started");
		_socket->flush();
	});
	_server->listen(QStringLiteral("__qtservice_testservice"));
	qDebug() << "listening:" << _server->isListening();

	//also: start basic TCP server if applicable
	auto socket = getSocket();
	if(socket >= 0) {
		_activatedServer = new QTcpServer(this);
		connect(_activatedServer, &QTcpServer::newConnection,
				this, [this]() {
			while(_activatedServer->hasPendingConnections()) {
				auto tcpSocket = _activatedServer->nextPendingConnection();
				tcpSocket->setParent(this);
				connect(tcpSocket, &QTcpSocket::readyRead,
						tcpSocket, [tcpSocket](){
					tcpSocket->write(tcpSocket->readAll());
				});
			}
		});
		_activatedServer->setSocketDescriptor(socket);
	}

	qDebug() << "start ready";
	return Synchronous;
}

Service::CommandMode TestService::onStop(int &exitCode)
{
	Q_UNUSED(exitCode);
	qDebug() << Q_FUNC_INFO;
	_stream << QByteArray("stopping");
	if(_socket) {
		_socket->flush();
		_socket->waitForBytesWritten(2500);
	}
	return Synchronous;
}

Service::CommandMode TestService::onReload()
{
	qDebug() << Q_FUNC_INFO;
	_stream << QByteArray("reloading");
	_socket->flush();
	return Synchronous;
}

Service::CommandMode TestService::onPause()
{
	qDebug() << Q_FUNC_INFO;
	_stream << QByteArray("pausing");
	_socket->flush();
	_socket->waitForBytesWritten(2500);
	return Synchronous;
}

Service::CommandMode TestService::onResume()
{
	qDebug() << Q_FUNC_INFO;
	_stream << QByteArray("resuming");
	_socket->flush();
	return Synchronous;
}

QVariant TestService::onCallback(const QByteArray &kind, const QVariantList &args)
{
	qDebug() << Q_FUNC_INFO << kind << args;
	_stream << kind << args;
	_socket->flush();
	return true;
}

bool TestService::verifyCommand(const QStringList &arguments)
{
	qDebug() << Q_FUNC_INFO << arguments;
	if(arguments.contains(QStringLiteral("--passive")))
		setTerminalMode(Service::ReadWritePassive);
	else
		setTerminalMode(Service::ReadWriteActive);
	return true;
}

void TestService::terminalConnected(Terminal *terminal)
{
	qDebug() << Q_FUNC_INFO << terminal->command();
	if(terminal->command().startsWith(QStringLiteral("stop")))
		quit();
	else if(terminal->terminalMode() == Service::ReadWriteActive) {
		connect(terminal, &Terminal::readyRead,
				terminal, [terminal](){
			qDebug() << Q_FUNC_INFO << terminal->readAll();
			terminal->disconnectTerminal();
		});
		terminal->write("name: ");
		terminal->requestLine();
	} else {
		connect(terminal, &Terminal::readyRead,
				terminal, [terminal](){
			auto data = terminal->readAll();
			qDebug() << Q_FUNC_INFO << data;
			terminal->write(data);
		});
	}
}
