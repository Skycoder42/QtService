#include "systemdservicebackend.h"
#include <chrono>
#include <csignal>
#include <unistd.h>

#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>
#include <systemd/sd-daemon.h>

#include <QtService/private/logging_p.h>

using namespace QtService;

SystemdServiceBackend::SystemdServiceBackend(QObject *parent) :
	ServiceBackend{parent}
{}

int SystemdServiceBackend::runService(QtService::Service *service, int &argc, char **argv, int flags)
{
	qInstallMessageHandler(SystemdServiceBackend::systemdMessageHandler);
	_service = service;

	try {
		auto pid = 0;
		if(findArg("stop", argc, argv, pid))
			return stop(pid);
		else if(findArg("reload", argc, argv, pid))
			return reload(pid);
		else
			return run(argc, argv, flags);
	} catch(int exitCode) {
		qCWarning(logQtService) << "Control commands must be used as \"<path/to/service> --backend systemd <command> $MAINPID\"";
		return exitCode;
	}
}

void SystemdServiceBackend::quitService()
{
	connect(_service, &Service::stopped,
			this, &SystemdServiceBackend::onStopped,
			Qt::UniqueConnection);
	sd_notify(false, "STOPPING=1");
	processServiceCommand(_service, StopCommand);
}

void SystemdServiceBackend::reloadService()
{
	sd_notify(false, "RELOADING=1");
	processServiceCommand(_service, ReloadCommand);
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
	switch(signal) {
	case SIGINT:
	case SIGTERM:
	case SIGQUIT:
		quitService();
		break;
	case SIGHUP:
		reloadService();
		break;
	case SIGTSTP:
		processServiceCommand(_service, PauseCommand);
		break;
	case SIGCONT:
		processServiceCommand(_service, ResumeCommand);
		break;
	case SIGUSR1:
		processServiceCommand(_service, UserCommand + 1);
		break;
	case SIGUSR2:
		processServiceCommand(_service, UserCommand + 2);
		break;
	default:
		ServiceBackend::signalTriggered(signal);
		break;
	}
}

void SystemdServiceBackend::sendWatchdog()
{
	sd_notify(false, "WATCHDOG=1");
}

void SystemdServiceBackend::onReady()
{
	sd_notify(false, "READY=1");
}

void SystemdServiceBackend::onStopped(int exitCode)
{
	if(_watchdogTimer)
		_watchdogTimer->stop();
	qApp->exit(exitCode);
}

int SystemdServiceBackend::run(int &argc, char **argv, int flags)
{
	//prepare the app
	QCoreApplication app{argc, argv, flags};
	prepareWatchdog(); //do as early as possible
	if(!preStartService(_service))
		return EXIT_FAILURE;

	connect(_service, &Service::started,
			this, &SystemdServiceBackend::onReady);
	connect(_service, &Service::reloaded,
			this, &SystemdServiceBackend::onReady);

	for(const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT, SIGUSR1, SIGUSR2})
		registerForSignal(signal);

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::Service*, _service),
							  Q_ARG(int, StartCommand));
	return QCoreApplication::exec();
}

int SystemdServiceBackend::stop(int pid)
{
	sd_notify(false, "STOPPING=1");
	qCDebug(logQtService) << "Sending signal SIGTERM to main process with pid" << pid;
	if(kill(pid, SIGTERM) == 0)
		return EXIT_SUCCESS;
	else
		return errno;
}

int SystemdServiceBackend::reload(int pid)
{
	sd_notify(false, "RELOADING=1");
	qCDebug(logQtService) << "Sending signal SIGHUP to main process with pid" << pid;
	if(kill(pid, SIGHUP) == 0)
		return EXIT_SUCCESS;
	else
		return errno;
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

bool SystemdServiceBackend::findArg(const char *command, int argc, char **argv, int &pid)
{
	for(auto i = 1; i < argc; i++) {
		if(qstrcmp(command, argv[i]) == 0) {
			if((i + 1) >= argc)
				throw EXIT_FAILURE;
			auto ok = false;
			pid = QByteArray(argv[i + 1]).toInt(&ok);
			if(ok)
				return true;
			else
				throw EXIT_FAILURE;
		}
	}
	return false;
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
