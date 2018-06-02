#include "terminalclient_p.h"
#include "terminalserver_p.h"
#include "terminal_p.h"
#include "service_p.h"
#include "servicecontrol.h"
#include "logging_p.h"
#include <iostream>
#include <QtCore/QThread>
#include <QtCore/QCoreApplication>
#include "qconsole.h"
#include "QCtrlSignals"
using namespace QtService;

TerminalClient::TerminalClient(Service *service) :
	QObject{service},
	_service{service}
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
	if(!verifyArgs())
		return EXIT_FAILURE;

	// enable signal handling for standard "quit" signals
	QCtrlSignalHandler::instance()->setAutoQuitActive(true);

	// setup the socket (not connect yet) and stdin/stderr handlers
	setupChannels();

	// if start -> use the control to ensure the service is running
	if(_service->startWithTerminal()) {
		if(!ensureServiceStarted())
			return EXIT_FAILURE;
	}

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

	_stream.setDevice(_socket);
	_stream << static_cast<int>(_mode) << _cmdArgs;
	_socket->flush();
	qCDebug(logQtService) << "Connected to service!";
	// write initial data for console
	if(_inConsole)
		consoleReady();
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
					readData = _inFile->readLine();
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
				_outFile->flush();
			}

			if(_stream.status() == QDataStream::ReadCorruptData) {
				qCCritical(logQtService) << "Invalid data on transmission stream. Canceling terminal";
				_exitFailed = true;
				_socket->disconnectFromServer();
			}
		}
	} else {
		_outFile->write(_socket->readAll());
		_outFile->flush();
	}
}

void TerminalClient::consoleReady()
{
	auto mBytes = _inConsole->bytesAvailable();
	if(mBytes > 0) {
		_socket->write(_inConsole->read(mBytes));
		_socket->flush();
	}
}

bool TerminalClient::verifyArgs()
{
	_cmdArgs = QCoreApplication::arguments();
	_cmdArgs.removeFirst();
	auto backendIndex = _cmdArgs.indexOf(QStringLiteral("--backend"));
	if(backendIndex >= 0) {
		// remove --backend <backend> (2 args)
		_cmdArgs.removeAt(backendIndex);
		_cmdArgs.removeAt(backendIndex);
	}
	_cmdArgs.removeOne(QStringLiteral("--terminal"));
	auto ok = _service->verifyCommand(_cmdArgs);
	// set mode after verify, as verify can change the mode
	_mode = _service->terminalMode();
	return ok;
}

bool TerminalClient::ensureServiceStarted()
{
	auto control = ServicePrivate::createLocalControl(_service->backend(), this);
	if(!control)
		qCWarning(logQtService) << "Unable to create control to ensure service is running";
	else if(!control->supportFlags().testFlag(ServiceControl::SupportsStart))
		qCWarning(logQtService) << "Service control does not support starting - cannot start service";
	else {
		control->setBlocking(true);
		auto canStatus = control->supportFlags().testFlag(ServiceControl::SupportsStatus);
		// start the service, depending on its status (if possible)
		if(!canStatus || control->status() == ServiceControl::ServiceStopped) {
			if(!control->start()) {
				qCCritical(logQtService).noquote() << control->error();
				return false;
			}
		}
		// ensure the service is running for controls that can check it
		if(canStatus) {
			auto waitCnt = control->supportFlags().testFlag(ServiceControl::SupportsBlocking) ? 0 : 15;
			while(control->status() != ServiceControl::ServiceRunning && waitCnt > 0) {
				QThread::sleep(1);
				--waitCnt;
			}
			if(control->status() != ServiceControl::ServiceRunning) {
				if(!control->error().isNull())
					qCCritical(logQtService).noquote() << control->error();
				else
					qCCritical(logQtService) << "Service did not reach the running state withing 15 seconds after starting it";
				return false;
			}
		}
	}

	return true;
}

void TerminalClient::setupChannels()
{
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
}

void TerminalClient::cerrMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
	std::cerr << qFormatLogMessage(type, context, message).toStdString() << std::endl;
}
