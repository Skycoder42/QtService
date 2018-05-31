#include "terminalclient_p.h"
#include "terminalserver_p.h"
#include "terminal_p.h"
#include <QtCore/QCoreApplication>
#include "qconsole.h"
#include "QCtrlSignals"
#include "logging_p.h"
#include <iostream>
using namespace QtService;

TerminalClient::TerminalClient(Service::TerminalMode mode, Service *service) :
	QObject{service},
	_service{service},
	_mode{mode}
{}

TerminalClient::~TerminalClient()
{
	if(_inConsole)
		_inConsole->close();
}

int TerminalClient::exec(int &argc, char **argv, int flags)
{
	//setup logging
#ifdef Q_OS_WIN
	qSetMessagePattern(QStringLiteral("["
									  "%{if-debug}Debug]    %{endif}"
									  "%{if-info}Info]     %{endif}"
									  "%{if-warning}Warning]  %{endif}"
									  "%{if-critical}Critical] %{endif}"
									  "%{if-fatal}Fatal]    %{endif}"
									  "%{if-category}%{category}: %{endif}"
									  "%{message}"));
#else
	qSetMessagePattern(QStringLiteral("["
									  "%{if-debug}\033[32mDebug\033[0m]    %{endif}"
									  "%{if-info}\033[36mInfo\033[0m]     %{endif}"
									  "%{if-warning}\033[33mWarning\033[0m]  %{endif}"
									  "%{if-critical}\033[31mCritical\033[0m] %{endif}"
									  "%{if-fatal}\033[35mFatal\033[0m]    %{endif}"
									  "%{if-category}%{category}: %{endif}"
									  "%{message}"));
#endif
	qInstallMessageHandler(TerminalClient::cerrMessageHandler);

	QCoreApplication app{argc, argv, flags};

	// verify args
	_cmdArgs = QCoreApplication::arguments();
	auto backendIndex = _cmdArgs.indexOf(QStringLiteral("--backend"));
	if(backendIndex >= 0) {
		// remove --backend <backend> (2 args)
		_cmdArgs.removeAt(backendIndex);
		_cmdArgs.removeAt(backendIndex);
	}
	_cmdArgs.removeOne(QStringLiteral("--terminal"));
	if(!_service->verifyCommand(_cmdArgs))
		return EXIT_FAILURE;

	// enable signal handling for standard "quit" signals
	QCtrlSignalHandler::instance()->setAutoQuitActive(true);

	_socket = new QLocalSocket{this};
	_outFile = QConsole::qStdOut(this);
	if(_mode == Service::ReadWriteActive)
		_inFile = QConsole::qStdIn(this);
	else {
		_inConsole = new QConsole{this};
		connect(_inConsole, &QConsole::readyRead,
				this, &TerminalClient::consoleReady);
	}

	connect(_socket, &QLocalSocket::connected,
			this, &TerminalClient::connected);
	connect(_socket, &QLocalSocket::disconnected,
			this, &TerminalClient::disconnected);
	connect(_socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
			this, &TerminalClient::error);
	connect(_socket, &QLocalSocket::readyRead,
			this, &TerminalClient::socketReady,
			Qt::QueuedConnection); //queued connection, because of "socket not ready" errors on win

	qCDebug(logQtService) << "Created terminal, waiting for service connection...";
	QMetaObject::invokeMethod(this, "doConnect", Qt::QueuedConnection);
	return app.exec();
}

void TerminalClient::doConnect()
{
	_socket->connectToServer(TerminalServer::serverName());
}

void TerminalClient::connected()
{
	if(_inConsole) {
		if(!_inConsole->open()) {
			qCCritical(logQtService).noquote() << "Failed to start stdin reader with error:" << _inConsole->errorString();
			_exitFailed = true;
			_socket->disconnectFromServer();
			return;
		}
	}

	auto args = QCoreApplication::arguments();
	auto backendIndex = args.indexOf(QStringLiteral("--backend"));
	if(backendIndex >= 0) {
		// remove --backend <backend> (2 args)
		args.removeAt(backendIndex);
		args.removeAt(backendIndex);
	}
	args.removeOne(QStringLiteral("--terminal"));

	_stream.setDevice(_socket);
	_stream << static_cast<int>(_mode) << args;
	_socket->flush();
	qCDebug(logQtService) << "Connected to service!";
}

void TerminalClient::disconnected()
{
	qCInfo(logQtService) << "Connection closed by service";
	_socket->close();
	qApp->exit(_exitFailed ? EXIT_FAILURE : EXIT_SUCCESS);
}

void TerminalClient::error(QLocalSocket::LocalSocketError socketError)
{
	if(socketError != QLocalSocket::PeerClosedError) {
		qCCritical(logQtService).noquote() << "Connection to service failed with error:"
										   << _socket->errorString();
		_socket->close();
		qApp->exit(EXIT_FAILURE);
	}
}

void TerminalClient::socketReady()
{
	if(_mode == Service::ReadWriteActive) {
		// in this mode, data is stream to "channel" it
		while(!_stream.atEnd()) {
			_stream.startTransaction();
			bool isCommand = false;
			_stream >> isCommand;
			if(isCommand) {
				int type = 0;
				int num = 0;
				_stream >> type;
				if(type == TerminalPrivate::MultiCharRequest)
					_stream >> num;
				// done with reading - commit
				if(!_stream.commitTransaction())
					break;

				// read the appropriate amount of data and immediatly return it (synchronously)
				QByteArray readData;
				switch(type) {
				case QtService::TerminalPrivate::CharRequest:
					num = 1;
					Q_FALLTHROUGH();
				case QtService::TerminalPrivate::MultiCharRequest:
					readData = _inFile->read(num);
					break;
				case QtService::TerminalPrivate::LineRequest:
					readData = _inFile->readLine(); //TODO if really blocking
					break;
				default:
					// hard error!!!
					_stream.setStatus(QDataStream::ReadCorruptData);
					break;
				}
				// and send it if present
				if(!readData.isEmpty()) {
					_socket->write(readData);
					_socket->flush();
				}
			} else {
				QByteArray data;
				_stream >> data;
				if(!_stream.commitTransaction())
					break;
				_outFile->write(data);
			}

			if(_stream.status() == QDataStream::ReadCorruptData) {
				qCCritical(logQtService) << "Invalid data on transmission stream. Canceling terminal";
				_exitFailed = true;
				_socket->disconnectFromServer();
			}
		}
	} else
		_outFile->write(_socket->readAll());
}

void TerminalClient::consoleReady()
{
	_socket->write(_inConsole->read(_inConsole->bytesAvailable()));
}

void TerminalClient::cerrMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
	std::cerr << qFormatLogMessage(type, context, message).toStdString() << std::endl;
}
