#ifndef QTSERVICE_TERMINAL_H
#define QTSERVICE_TERMINAL_H

#include <QtCore/qiodevice.h>
#include <QtCore/qscopedpointer.h>

#include "QtService/qtservice_global.h"
#include "QtService/service.h"

namespace QtService {

class TerminalPrivate;
class TerminalAwaitablePrivate;
class Q_SERVICE_EXPORT Terminal : public QIODevice //TODO copy doc from backproc
{
	Q_OBJECT

	Q_PROPERTY(QtService::Service::TerminalMode terminalMode READ terminalMode CONSTANT)
	Q_PROPERTY(QStringList command READ command CONSTANT)
	Q_PROPERTY(bool autoDelete READ isAutoDelete WRITE setAutoDelete NOTIFY autoDeleteChanged)

public:
	class Q_SERVICE_EXPORT Awaitable
	{
	public:
		enum SpecialReads : qint64 {
			ReadLine = 0,
			ReadSingle = 1
		};

		Awaitable(Terminal *terminal, qint64 readCnt = ReadSingle);
		Awaitable(Awaitable &&other);
		Awaitable &operator=(Awaitable &&other);
		~Awaitable();

		using type = QByteArray;
		void prepare(std::function<void()> resume);
		type &&result();

	private:
		QScopedPointer<TerminalAwaitablePrivate> d;
	};

	explicit Terminal(TerminalPrivate *d_ptr, QObject *parent = nullptr);
	~Terminal() override;

	bool isSequential() const override;
	void close() override;
	bool atEnd() const override;
	qint64 bytesAvailable() const override;
	qint64 bytesToWrite() const override;
	bool canReadLine() const override;
	bool waitForReadyRead(int msecs) override;
	bool waitForBytesWritten(int msecs) override;

	Service::TerminalMode terminalMode() const;
	QStringList command() const;
	bool isAutoDelete() const;

	//awaitables
	Awaitable awaitChar();
	Awaitable awaitChars(qint64 num);
	Awaitable awaitLine();

public Q_SLOTS:
	void disconnectTerminal();

	void requestChar();
	void requestChars(qint64 num);
	void requestLine();

	void writeLine(const QByteArray &line, bool flush = true);
	void flush();

	void setAutoDelete(bool autoDelete);

Q_SIGNALS:
	void terminalDisconnected();
	void terminalError(int errorCode);

	void autoDeleteChanged(bool autoDelete);

protected:
	qint64 readData(char *data, qint64 maxlen) override;
	qint64 readLineData(char *data, qint64 maxlen) override;
	qint64 writeData(const char *data, qint64 len) override;

private:
	TerminalPrivate *d;

	bool open(OpenMode mode) override;
};

}

#endif // QTSERVICE_TERMINAL_H
