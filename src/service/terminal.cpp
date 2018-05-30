#include "terminal.h"
#include "terminal_p.h"
#include "logging_p.h"
using namespace QtService;

Terminal::Terminal(TerminalPrivate *d_ptr, QObject *parent) :
	QIODevice{parent},
	d{d_ptr}
{
	d->setParent(this);

	QIODevice::OpenMode mode;
	switch(d->terminalMode) {
	case QtService::Service::ReadOnly:
		mode = QIODevice::WriteOnly; // a read only terminal means the service can only write
		break;
	case QtService::Service::WriteOnly:
		mode = QIODevice::ReadOnly; // a write only terminal means the service can only read
		break;
	case QtService::Service::ReadWritePassive:
	case QtService::Service::ReadWriteActive:
		mode = QIODevice::ReadWrite;
		break;
	}
	// open as combination of theoretical mode, limited to actual mode, but unbuffered
	QIODevice::open((mode & d->socket->openMode()) | QIODevice::Unbuffered);

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

Service::TerminalMode Terminal::terminalMode() const
{
	return d->terminalMode;
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

void Terminal::requestChar()
{
	if(d->terminalMode != Service::ReadWriteActive) {
		qCWarning(logQtService) << "The request methods are only avialable for QtService::Service::ReadWriteActive terminal mode - doing nothing!";
		return;
	}
	d->commandStream << true
					 << TerminalPrivate::CharRequest;
	d->socket->flush();
}

void Terminal::requestChars(qint64 num)
{
	if(d->terminalMode != Service::ReadWriteActive) {
		qCWarning(logQtService) << "The request methods are only avialable for QtService::Service::ReadWriteActive terminal mode - doing nothing!";
		return;
	}
	d->commandStream << true
					 << TerminalPrivate::MultiCharRequest
					 << num;
	d->socket->flush();
}

void Terminal::requestLine()
{
	if(d->terminalMode != Service::ReadWriteActive) {
		qCWarning(logQtService) << "The request methods are only avialable for QtService::Service::ReadWriteActive terminal mode - doing nothing!";
		return;
	}
	d->commandStream << true
					 << TerminalPrivate::LineRequest;
	d->socket->flush();
}

void Terminal::writeLine(const QByteArray &line, bool flush)
{
	write(line + '\n');
	if(flush)
		this->flush();
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
	if(d->terminalMode == Service::ReadWriteActive) {
		if(len > std::numeric_limits<int>::max()) {
			for(qint64 lIndex = 0; lIndex < len; lIndex += std::numeric_limits<int>::max()) {
				auto writeData = data + lIndex;
				auto writeLen = (len - lIndex) > std::numeric_limits<int>::max() ?
									std::numeric_limits<int>::max() :
									static_cast<int>(len - lIndex);
				d->commandStream << false << QByteArray::fromRawData(writeData, writeLen);
			}
		} else
			d->commandStream << false << QByteArray::fromRawData(data, static_cast<int>(len));
		return len;
	} else
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
		emit terminalReady(this, false);
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
			emit terminalReady(this, false);
			socket->close();
		}
	}
}

void TerminalPrivate::readyRead()
{
	if(isLoading) {
		commandStream.startTransaction();
		int tMode;
		commandStream >> tMode >> command;
		if(commandStream.commitTransaction()) {
			terminalMode = static_cast<Service::TerminalMode>(tMode);
			isLoading = false;
			//disconnect all but "disconencted" - that one is needed for auto-delete
			disconnect(socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
					   this, &TerminalPrivate::error);
			disconnect(socket, &QLocalSocket::readyRead,
					   this, &TerminalPrivate::readyRead);
			emit terminalReady(this, true);
		}
	}
}
