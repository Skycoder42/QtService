#include "echoservice.h"
#include <QDebug>
#include <QTcpSocket>
#include <QTimer>

EchoService::EchoService(int &argc, char **argv) :
	Service(argc, argv)
{}

bool EchoService::preStart()
{
	qDebug() << Q_FUNC_INFO;
	qInfo() << "Service running with backend:" << backend();

	addCallback("SIGUSR1", [](){
		qDebug() << "SIGUSR1";
	});
	addCallback("SIGUSR2", [](){
		qDebug() << "SIGUSR2";
		return 42;
	});

	return true;
}

QtService::Service::CommandMode EchoService::onStart()
{
	qDebug() << Q_FUNC_INFO;
	_server = new QTcpServer(this);
	connect(_server, &QTcpServer::newConnection,
			this, &EchoService::newConnection);

	auto socket = getSocket();
	auto ok = false;
	if(socket >= 0) {
		qDebug() << "Using activated socket descriptor:" << socket;
		ok = _server->setSocketDescriptor(socket);
	} else {
		qDebug() << "No sockets activated - creating normal socket";
		ok = _server->listen();
	}
	if(ok)
		qInfo() << "Started echo server on port" << _server->serverPort();
	else {
		qCritical().noquote() << "Failed to start server with error" << _server->errorString();
		qApp->exit(EXIT_FAILURE);
	}

	return Synchronous;
}

QtService::Service::CommandMode EchoService::onStop(int &exitCode)
{
	Q_UNUSED(exitCode)
	qDebug() << Q_FUNC_INFO;
	_server->close();
	return Synchronous;
}

QtService::Service::CommandMode EchoService::onReload()
{
	qDebug() << Q_FUNC_INFO;
	_server->close();
	if(_server->listen())
		qInfo() << "Restarted echo server on port" << _server->serverPort();
	else {
		qCritical().noquote() << "Failed to restart server with error" << _server->errorString();
		qApp->exit(EXIT_FAILURE);
	}

	return Synchronous;
}

QtService::Service::CommandMode EchoService::onPause()
{
	qDebug() << Q_FUNC_INFO;
	_server->pauseAccepting();
	return Synchronous;
}

QtService::Service::CommandMode EchoService::onResume()
{
	qDebug() << Q_FUNC_INFO;
	_server->resumeAccepting();
	return Synchronous;
}

void EchoService::newConnection()
{
	while(_server->hasPendingConnections()) {
		auto socket = _server->nextPendingConnection();
		socket->setParent(this);
		connect(socket, &QTcpSocket::readyRead,
				socket, [socket]() {
			auto msg = socket->readAll();
			qDebug() << host(socket) << "Echoing:" << msg;
			socket->write(msg);
		});
		connect(socket, &QTcpSocket::disconnected,
				socket, [socket]() {
			qInfo() << host(socket) << "disconnected";
			socket->close();
			socket->deleteLater();
		});
		connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
				socket, [socket](QAbstractSocket::SocketError error) {
			qWarning() << host(socket) << "Socket-Error[" << error << "]:" << qUtf8Printable(socket->errorString());
		});
		qInfo() << host(socket) << "connected";
	}
}

QByteArray EchoService::host(QTcpSocket *socket)
{
	return (QLatin1Char('<') + socket->peerAddress().toString() + QLatin1Char(':') + QString::number(socket->peerPort()) + QLatin1Char('>')).toUtf8();
}
