#include "echoservice.h"
#include <QDebug>
#include <QTcpSocket>

EchoService::EchoService(int &argc, char **argv) :
	Service(argc, argv)
{}

bool EchoService::preStart()
{
	qDebug() << Q_FUNC_INFO;
	return true;
}

void EchoService::start()
{
	qDebug() << Q_FUNC_INFO;
	_server = new QTcpServer(this);
	connect(_server, &QTcpServer::newConnection,
			this, &EchoService::newConnection);

	if(_server->listen())
		qInfo() << "Started echo server on port" << _server->serverPort();
	else {
		qCritical() << "Failed to start server with error" << _server->errorString();
		qApp->exit(EXIT_FAILURE);
	}
}

void EchoService::stop()
{
	qDebug() << Q_FUNC_INFO;
	_server->close();
}

void EchoService::pause()
{
	qDebug() << Q_FUNC_INFO;
	_server->pauseAccepting();
}

void EchoService::resume()
{
	qDebug() << Q_FUNC_INFO;
	_server->resumeAccepting();
}

void EchoService::reload()
{
	qDebug() << Q_FUNC_INFO;
	_server->close();
	if(_server->listen())
		qInfo() << "Restarted echo server on port" << _server->serverPort();
	else {
		qCritical() << "Failed to restart server with error" << _server->errorString();
		qApp->exit(EXIT_FAILURE);
	}
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
			qWarning() << host(socket) << "Socket-Error[" << error << "]:" << socket->errorString();
		});
		qInfo() << host(socket) << "connected";
	}
}

QByteArray EchoService::host(QTcpSocket *socket)
{
	return (QLatin1Char('<') + socket->peerAddress().toString() + QLatin1Char(':') + QString::number(socket->peerPort()) + QLatin1Char('>')).toUtf8();
}
