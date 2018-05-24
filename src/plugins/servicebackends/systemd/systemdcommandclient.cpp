#include "systemdcommandclient.h"
#include <QtService/private/logging_p.h>

const QByteArray SystemdCommandClient::StopCommand{"stop"};
const QByteArray SystemdCommandClient::ReloadCommand{"reload"};
const QByteArray SystemdCommandClient::DoneCommand{"done"};

SystemdCommandClient::SystemdCommandClient(QLocalSocket *socket, QObject *parent) :
	QObject{parent},
	_socket{socket}
{
	_socket->setParent(this);
	connect(_socket, &QLocalSocket::disconnected,
			this, &SystemdCommandClient::disconnected);
	connect(_socket, &QLocalSocket::readyRead,
			this, &SystemdCommandClient::readyRead);
	connect(_socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
			this, &SystemdCommandClient::error);
}

void SystemdCommandClient::receiveFor(SystemdServiceBackend *backend)
{
	_isServer = true;
	_backend = backend;

	_stream.setDevice(_socket);
}

void SystemdCommandClient::sendCommand(const QString &socketPath, const QByteArray &command)
{
	_isServer = false;
	_command = command;

	connect(_socket, &QLocalSocket::connected,
			this, &SystemdCommandClient::connected);
	_socket->connectToServer(socketPath);
}

void SystemdCommandClient::operationDone()
{
	using namespace std::chrono_literals;
	_stream << DoneCommand;
	_closing = true;
	QTimer::singleShot(5s, _socket, &QLocalSocket::disconnectFromServer);
}

void SystemdCommandClient::connected()
{
	_stream.setDevice(_socket);
	_stream << _command;

	if(_command == StopCommand)
		_closing = true; //set closing now, as for a quitting service disconnecting without a reply is ok
}

void SystemdCommandClient::disconnected()
{
	_socket->close();
	if(!_isServer)
		qApp->exit(_closing ? EXIT_SUCCESS : EXIT_FAILURE);
	deleteLater();
}

void SystemdCommandClient::readyRead()
{
	forever {
		_stream.startTransaction();
		QByteArray message;
		_stream >> message;
		if(!_stream.commitTransaction())
			break;

		if(_isServer)
			handlerServerCommand(message);
		else
			handlerClientCommand(message);
	}
}

void SystemdCommandClient::error(QLocalSocket::LocalSocketError socketError)
{
	if(socketError != QLocalSocket::PeerClosedError || !_closing) {
		qCWarning(logQtService).noquote() << "Socket error on command socket:" << _socket->errorString();
		_socket->disconnectFromServer();
	}
}

void SystemdCommandClient::handlerServerCommand(const QByteArray &command)
{
	if(command == StopCommand)
		_backend->quitService();
	else if(command == ReloadCommand)
		_backend->reloadService();
	else {
		qCWarning(logQtService) << "Received invalid command:" << command;
		_closing = true;
		_socket->disconnectFromServer();
	}
	//TODO report back result
}

void SystemdCommandClient::handlerClientCommand(const QByteArray &command)
{
	if(command == DoneCommand)
		qCDebug(logQtService) << "Command completed successfully";
	else
		qCWarning(logQtService) << "Received invalid command:" << command;
	_closing = true;
	_socket->disconnectFromServer();
}
