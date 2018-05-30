#include "testservice.h"

#include <QTimer>
#include <QTcpSocket>
using namespace QtService;

TestService::TestService(int &argc, char **argv) :
	Service{argc, argv}
{}

bool TestService::preStart()
{
	return true;
}

Service::CommandMode TestService::onStart()
{
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
	});
	_server->listen(runtimeDir().absoluteFilePath(QStringLiteral("__qtservice_testservice")));
	qDebug() << "listening:" << _server->isListening();
	QTimer::singleShot(10000, this, [this](){
		qDebug() << "timeout";
		if(!_socket)
			qApp->quit();
	});

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
	_stream << QByteArray("stopping");
	if(_socket)
		exitCode = _socket->waitForBytesWritten(5000) ? EXIT_SUCCESS : EXIT_FAILURE;
	return Synchronous;
}

Service::CommandMode TestService::onReload()
{
	_stream << QByteArray("reloading");
	return Synchronous;
}

Service::CommandMode TestService::onPause()
{
	_stream << QByteArray("pausing");
	_socket->waitForBytesWritten(5000);
	return Synchronous;
}

Service::CommandMode TestService::onResume()
{
	_stream << QByteArray("resuming");
	return Synchronous;
}
