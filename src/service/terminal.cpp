#include "terminal.h"
#include "terminal_p.h"
#include "logging_p.h"
using namespace QtService;

Terminal::Terminal(TerminalPrivate *d_ptr, QObject *parent) :
	QIODevice{parent},
	d{d_ptr}
{
	d->setParent(this);
	QIODevice::open(d->socket->openMode() | QIODevice::Unbuffered);

	connect(d->socket, &QLocalSocket::disconnected,
			this, &Terminal::terminalDisconnected);
	connect(d->socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
			this, [this](QLocalSocket::LocalSocketError e) {
		if(e != QLocalSocket::PeerClosedError) {
			setErrorString(d->socket->errorString());
			emit terminalError(e);
		}
	});

	connect(d->socket, &QLocalSocket::channelReadyRead,
			this, &Terminal::channelReadyRead);
	connect(d->socket, &QLocalSocket::readyRead,
			this, &Terminal::readyRead);
}

Terminal::~Terminal() = default;

bool Terminal::isSequential() const
{
	return true;
}

void Terminal::close()
{
	d->socket->close();
	QIODevice::close();
}

bool Terminal::atEnd() const
{
	return d->socket->atEnd() && QIODevice::atEnd();
}

qint64 Terminal::bytesAvailable() const
{
	return QIODevice::bytesAvailable() + d->socket->bytesAvailable();
}

qint64 Terminal::bytesToWrite() const
{
	return QIODevice::bytesToWrite() + d->socket->bytesToWrite();
}

bool Terminal::canReadLine() const
{
	return d->socket->canReadLine() || QIODevice::canReadLine();
}

bool Terminal::waitForReadyRead(int msecs)
{
	return d->socket->waitForReadyRead(msecs);
}

bool Terminal::waitForBytesWritten(int msecs)
{
	return d->socket->waitForBytesWritten(msecs);
}

QStringList Terminal::command() const
{
	return d->command;
}

bool Terminal::isAutoDelete() const
{
	return d->autoDelete;
}

void Terminal::disconnectTerminal()
{
	d->socket->disconnectFromServer();
}

void Terminal::writeLine(const QByteArray &line, bool flush)
{
	d->socket->write(line + '\n');
	if(flush)
		d->socket->flush();
}

void Terminal::flush()
{
	d->socket->flush();
}

void Terminal::setAutoDelete(bool autoDelete)
{
	if (d->autoDelete == autoDelete)
		return;

	d->autoDelete = autoDelete;
	emit autoDeleteChanged(d->autoDelete);
}

qint64 Terminal::readData(char *data, qint64 maxlen)
{
	return d->socket->read(data, maxlen);
}

qint64 Terminal::readLineData(char *data, qint64 maxlen)
{
	return d->socket->readLine(data, maxlen);
}

qint64 Terminal::writeData(const char *data, qint64 len)
{
	return d->socket->write(data, len);
}

bool Terminal::open(QIODevice::OpenMode mode)
{
	Q_UNUSED(mode);
	Q_ASSERT_X(false, Q_FUNC_INFO, "QIODevice::open must not be called on a terminal!");
	return false;
}

// ------------- Private Implementation -------------

TerminalPrivate::TerminalPrivate(QLocalSocket *socket, QObject *parent) :
	QObject{parent},
	socket{socket},
	commandStream{socket}
{
	socket->setParent(this);

	connect(socket, &QLocalSocket::disconnected,
			this, &TerminalPrivate::disconnected);
	connect(socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
			this, &TerminalPrivate::error);
	connect(socket, &QLocalSocket::readyRead,
			this, &TerminalPrivate::readyRead);
}

void TerminalPrivate::disconnected()
{
	if(isLoading) {
		isLoading = false;
		emit statusLoadComplete(this, false);
		socket->close();
	} else if(autoDelete && parent())
		parent()->deleteLater();
}

void TerminalPrivate::error()
{
	if(isLoading) {
		qCWarning(logQtService).noquote() << "Terminal closed due to connection error while loading terminal status:"
										  << socket->errorString();
		if(socket->state() == QLocalSocket::ConnectedState)
			socket->disconnectFromServer();
		else {
			isLoading = false;
			emit statusLoadComplete(this, false);
			socket->close();
		}
	}
}

void TerminalPrivate::readyRead()
{
	if(isLoading) {
		commandStream.startTransaction();
		commandStream >> command;
		if(commandStream.commitTransaction()) {
			isLoading = false;
			//disconnect all but "disconencted" - that one is needed for auto-delete
			disconnect(socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
					   this, &TerminalPrivate::error);
			disconnect(socket, &QLocalSocket::readyRead,
					   this, &TerminalPrivate::readyRead);
			emit statusLoadComplete(this, true);
		}
	}
}
