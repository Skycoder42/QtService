#ifndef QTSERVICE_TERMINAL_H
#define QTSERVICE_TERMINAL_H

#include <QtCore/qiodevice.h>
#include <QtCore/qscopedpointer.h>

#include "QtService/qtservice_global.h"
#include "QtService/service.h"

namespace QtService {

class TerminalPrivate;
class TerminalAwaitablePrivate;
//! Represents a connection to a console terminal connected to the service
class Q_SERVICE_EXPORT Terminal : public QIODevice
{
	Q_OBJECT

	//! The I/O-mode the terminal operates in
	Q_PROPERTY(QtService::Service::TerminalMode terminalMode READ terminalMode CONSTANT)
	//! The command line arguments that have been used to create this terminal
	Q_PROPERTY(QStringList command READ command CONSTANT)
	//! If true, the terminal will delete itself as soon as the connection has been closed
	Q_PROPERTY(bool autoDelete READ isAutoDelete WRITE setAutoDelete NOTIFY autoDeleteChanged)

public:
	//! A helper class to be used with [QtCoroutines](https://github.com/Skycoder42/QtCoroutines) to await io from a coroutine
	class Q_SERVICE_EXPORT Awaitable
	{
	public:
		//! Special read modes
		enum SpecialReads : qint64 {
			ReadLine = 0, //!< Read until the next newline (QIODevice::readLine)
			ReadSingle = 1 //!< Read only a single character
		};

		//! Create an awaitable for the given terminal and specify how much data should be read
		Awaitable(Terminal *terminal, qint64 readCnt = ReadSingle);
		//! Move constructor
		Awaitable(Awaitable &&other) noexcept;
		//! Move assignment operator
		Awaitable &operator=(Awaitable &&other) noexcept;
		~Awaitable();

		//! @private
		using type = QByteArray;
		//! @private
		void prepare(std::function<void()> resume);
		//! @private
		type result();

	private:
		QScopedPointer<TerminalAwaitablePrivate> d;
	};

	//! @private
	explicit Terminal(TerminalPrivate *d_ptr, QObject *parent = nullptr);
	~Terminal() override;

	//! @inherit{QIODevice::isSequential}
	bool isSequential() const override;
	//! @inherit{QIODevice::close}
	void close() override;
	//! @inherit{QIODevice::atEnd}
	bool atEnd() const override;
	//! @inherit{QIODevice::bytesAvailable}
	qint64 bytesAvailable() const override;
	//! @inherit{QIODevice::bytesToWrite}
	qint64 bytesToWrite() const override;
	//! @inherit{QIODevice::canReadLine}
	bool canReadLine() const override;
	//! @inherit{QIODevice::waitForReadyRead}
	bool waitForReadyRead(int msecs) override;
	//! @inherit{QIODevice::waitForBytesWritten}
	bool waitForBytesWritten(int msecs) override;

	//! @readAcFn{Terminal::terminalMode}
	Service::TerminalMode terminalMode() const;
	//! @readAcFn{Terminal::command}
	QStringList command() const;
	//! @readAcFn{Terminal::autoDelete}
	bool isAutoDelete() const;

	//awaitables
	//! Await a single character
	Awaitable awaitChar();
	//! Await a given number of characters
	Awaitable awaitChars(qint64 num);
	//! Await a line of characters
	Awaitable awaitLine();

public Q_SLOTS:
	//! Disconnects the terminal from the client
	void disconnectTerminal();

	//! Request a single character from the terminal
	void requestChar();
	//! Request a given number of charactersr from the terminal
	void requestChars(qint64 num);
	//! Request a line of characters from the terminal
	void requestLine();

	//! Writes the given line, appends a newline and optionally flushes
	void writeLine(const QByteArray &line, bool flush = true);
	//! Flushes the terminal
	void flush();

	//! @writeAcFn{Terminal::autoDelete}
	void setAutoDelete(bool autoDelete);

Q_SIGNALS:
	//! Will be emitted after the terminal has been disconnected
	void terminalDisconnected();
	//! Will be emitted if an error occured. Use QIODevice::errorString to get the text
	void terminalError(int errorCode);

	//! @notifyAcFn{Terminal::autoDelete}
	void autoDeleteChanged(bool autoDelete);

protected:
	//! @inherit{QIODevice::readData}
	qint64 readData(char *data, qint64 maxlen) override;
	//! @inherit{QIODevice::readLineData}
	qint64 readLineData(char *data, qint64 maxlen) override;
	//! @inherit{QIODevice::writeData}
	qint64 writeData(const char *data, qint64 len) override;

private:
	TerminalPrivate *d;

	bool open(OpenMode mode) override;
};

}

#endif // QTSERVICE_TERMINAL_H
