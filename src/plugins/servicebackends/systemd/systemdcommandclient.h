#ifndef SYSTEMDCOMMANDCLIENT_H
#define SYSTEMDCOMMANDCLIENT_H

#include <QtCore/QObject>
#include <QtCore/QDataStream>
#include <QtNetwork/QLocalSocket>

#include "systemdservicebackend.h"

class SystemdCommandClient : public QObject
{
	Q_OBJECT

public:
	static const QByteArray StopCommand;
	static const QByteArray ReloadCommand;
	static const QByteArray DoneCommand;

	explicit SystemdCommandClient(QLocalSocket *socket, QObject *parent = nullptr);

	void receiveFor(SystemdServiceBackend *backend);
	void sendCommand(const QString &socketPath, const QByteArray &command);

public Q_SLOTS:
	void operationDone();

private Q_SLOTS:
	void connected();
	void disconnected();
	void readyRead();
	void error(QLocalSocket::LocalSocketError socketError);

private:
	QLocalSocket *_socket;
	QDataStream _stream;
	bool _isServer = false;
	bool _closing = false;

	// client
	QByteArray _command;

	// service
	SystemdServiceBackend *_backend = nullptr;

	void handlerServerCommand(const QByteArray &command);
	void handlerClientCommand(const QByteArray &command);
};

#endif // SYSTEMDCOMMANDCLIENT_H
