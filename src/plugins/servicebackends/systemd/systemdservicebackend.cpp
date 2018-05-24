#include "systemdservicebackend.h"
#include <chrono>
#include <csignal>
#include <unistd.h>

#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>
#include <systemd/sd-daemon.h>

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtService/private/logging_p.h>

#include "systemdcommandclient.h"

using namespace QtService;

SystemdServiceBackend::SystemdServiceBackend(QObject *parent) :
	ServiceBackend{parent}
{}

int SystemdServiceBackend::runService(QtService::Service *service, int &argc, char **argv, int flags)
{
	qInstallMessageHandler(SystemdServiceBackend::systemdMessageHandler);
	QCoreApplication app{argc, argv, flags};
	_service = service;

	if(QCoreApplication::arguments().contains(QStringLiteral("stop")))
		return stop();
	else if(QCoreApplication::arguments().contains(QStringLiteral("reload")))
		return reload();
	else
		return run();
}

void SystemdServiceBackend::quitService()
{
	connect(_service, &Service::stopped,
			qApp, &QCoreApplication::exit,
			Qt::UniqueConnection); //TODO close socket connection to signale completition + stop watchdog
	sd_notify(false, "STOPPING=1");
	stopService(_service);
}

void SystemdServiceBackend::reloadService()
{
	sd_notify(false, "RELOADING=1");
	processServiceCommand(_service, Service::ReloadCode);
	sd_notify(false, "READY=1"); //TODO make async
}

QHash<int, QByteArray> SystemdServiceBackend::getActivatedSockets()
{
	char **names = nullptr;
	auto cnt = sd_listen_fds_with_names(false, &names);
	if(cnt <= 0) {
		if(names)
			free(names);
		return {};
	}

	QHash<int, QByteArray> sockets;
	sockets.reserve(cnt);
	for(auto i = 0; i < cnt; i++) {
		QByteArray name;
		if(names[i])
			name = names[i];
		sockets.insert(SD_LISTEN_FDS_START + i, name);
	}
	free(names);
	return sockets;
}

void SystemdServiceBackend::signalTriggered(int signal)
{
	qCWarning(logQtService) << "Systemd service should not be controlled via signals!";
	switch(signal) {
	case SIGHUP:
		reloadService();
		break;
	case SIGTSTP:
		processServiceCommand(_service, Service::PauseCode);
		break;
	case SIGCONT:
		processServiceCommand(_service, Service::ResumeCode);
		break;
	case SIGINT:
	case SIGTERM:
	case SIGQUIT:
		quitService();
		break;
	default:
		ServiceBackend::signalTriggered(signal);
		break;
	}
}

void SystemdServiceBackend::performStart()
{
	// create the command server
	_commandServer = new QLocalServer{this};
	connect(_commandServer, &QLocalServer::newConnection,
			this, &SystemdServiceBackend::newConnection);
	//part 1: try to listen, and optionally remove a stale socket if applicable
	auto sName = getSocketName();
	if(!_commandServer->listen(sName) &&
	   _commandServer->serverError() == QAbstractSocket::AddressInUseError) {
		qCWarning(logQtService) << "Socket" << sName << "is already in use - trying to free it...";
		if(QLocalServer::removeServer(sName)) {
			qCInfo(logQtService) << "Successfully freed socket. Trying to listen once more...";
			_commandServer->listen(sName);
		}
	}
	// part 2: check the listen result for logging
	if(_commandServer->isListening())
		qCDebug(logQtService) << "Created daemon socket as" << _commandServer->fullServerName();
	else
		qCCritical(logQtService).noquote() << "Failed to start daemon socket with error:" << _commandServer->errorString();

	// perform the actual service start
	startService(_service);
	sd_notify(false, "READY=1");
}

void SystemdServiceBackend::sendWatchdog()
{
	sd_notify(false, "WATCHDOG=1");
}

void SystemdServiceBackend::newConnection()
{
	while(_commandServer->hasPendingConnections()) {
		auto client = new SystemdCommandClient {
			_commandServer->nextPendingConnection(),
			this
		};
		client->receiveFor(this);
	}
}

int SystemdServiceBackend::run()
{
	//prepare the app
	prepareWatchdog(); //do as early as possible
	if(!preStartService(_service))
		return EXIT_FAILURE;

	for(const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT})
		registerForSignal(signal);

	// start the eventloop
	QMetaObject::invokeMethod(this, "performStart", Qt::QueuedConnection);
	return QCoreApplication::exec();
}

int SystemdServiceBackend::stop()
{
	auto client = new SystemdCommandClient {
		new QLocalSocket{},
		this
	};
	client->sendCommand(getSocketName(), SystemdCommandClient::StopCommand);
	return QCoreApplication::exec();
}

int SystemdServiceBackend::reload()
{
	auto client = new SystemdCommandClient {
		new QLocalSocket{},
		this
	};
	client->sendCommand(getSocketName(), SystemdCommandClient::ReloadCommand);
	return QCoreApplication::exec();
}

void SystemdServiceBackend::prepareWatchdog()
{
	using namespace std::chrono;
	using namespace std::chrono_literals;
	uint64_t usec = 0;
	if(sd_watchdog_enabled(false, &usec) > 0) {
		auto mSecs = duration_cast<milliseconds>(microseconds{usec} / 2);
		if(mSecs.count() <= 1) {
			qCWarning(logQtService) << "Watchdog timout <= 1 millisecond - QTimer does not support intervals that small!";
			mSecs = 1ms;
		}

		_watchdogTimer = new QTimer(this);
		_watchdogTimer->setTimerType(Qt::PreciseTimer);
		_watchdogTimer->setInterval(mSecs);
		connect(_watchdogTimer, &QTimer::timeout,
				this, &SystemdServiceBackend::sendWatchdog);
		_watchdogTimer->start();
		qCDebug(logQtService) << "Enabled watchdog with an interval of" << mSecs.count() << "ms";
	}
}

QString SystemdServiceBackend::getSocketName()
{
	QDir runtimeDir;
	if(::geteuid() == 0)
		runtimeDir = QStringLiteral("/run");
	else
		runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
	auto appName = QCoreApplication::applicationName();
	auto setPerms = !runtimeDir.exists(appName); // set perms if dir does not exist
	if(!runtimeDir.mkpath(appName) || !runtimeDir.cd(appName)) {
		qCWarning(logQtService) << "Failed to access default runtime directory - falling back to the temporary dir";
		runtimeDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
		// dont check for errors here
		runtimeDir.mkpath(appName);
		runtimeDir.cd(appName);
	}
	if(setPerms)
		QFile::setPermissions(runtimeDir.absolutePath(), QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser);
	return runtimeDir.absoluteFilePath(QStringLiteral("qdaemon.socket"));
}

void SystemdServiceBackend::systemdMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
	auto formattedMessage = qFormatLogMessage(type, context, message);

	int priority = LOG_INFO; // Informational
	switch (type) {
	case QtDebugMsg:
		priority = LOG_DEBUG; // Debug-level messages
		break;
	case QtInfoMsg:
		priority = LOG_INFO; // Informational conditions
		break;
	case QtWarningMsg:
		priority = LOG_WARNING; // Warning conditions
		break;
	case QtCriticalMsg:
		priority = LOG_CRIT; // Critical conditions
		break;
	case QtFatalMsg:
		priority = LOG_ALERT; // Action must be taken immediately
		break;
	}

	sd_journal_send("MESSAGE=%s",     formattedMessage.toUtf8().constData(),
					"PRIORITY=%i",    priority,
					"CODE_FUNC=%s",   context.function ? context.function : "unknown",
					"CODE_LINE=%d",   context.line,
					"CODE_FILE=%s",   context.file ? context.file : "unknown",
					"QT_CATEGORY=%s", context.category ? context.category : "unknown",
					NULL);
}
