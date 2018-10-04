#include "systemdservicebackend.h"
#include "systemdserviceplugin.h"
#include <chrono>
#include <csignal>
#include <unistd.h>

#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>
#include <systemd/sd-daemon.h>

using namespace QtService;

SystemdServiceBackend::SystemdServiceBackend(Service *service) :
	ServiceBackend{service}
{}

int SystemdServiceBackend::runService(int &argc, char **argv, int flags)
{
	qInstallMessageHandler(SystemdServiceBackend::systemdMessageHandler);
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
	connect(service(), &Service::stopped,
			this, &SystemdServiceBackend::onStopped,
			Qt::UniqueConnection);
	sd_notify(false, "STOPPING=1");
	processServiceCommand(StopCommand);
}

void SystemdServiceBackend::reloadService()
{
	sd_notify(false, "RELOADING=1");
	processServiceCommand(ReloadCommand);
}

QList<int> SystemdServiceBackend::getActivatedSockets(const QByteArray &name)
{
	if(_sockets.isEmpty()) {
		char **names = nullptr;
		auto cnt = sd_listen_fds_with_names(false, &names);
		if(cnt > 0) {
			_sockets.reserve(cnt);
			for(auto i = 0; i < cnt; i++) {
				QByteArray sockName;
				if(names[i])
					sockName = names[i];
				_sockets.insert(sockName, SD_LISTEN_FDS_START + i);
			}
		}
		if(names)
			free(names);
	}

	if(name.isNull())
		return _sockets.isEmpty() ? QList<int>{} : QList<int>{SD_LISTEN_FDS_START};
	else
		return _sockets.values(name);
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
		processServiceCommand(PauseCommand);
		break;
	case SIGCONT:
		processServiceCommand(ResumeCommand);
		break;
	case SIGUSR1:
		processServiceCallback("SIGUSR1");
		break;
	case SIGUSR2:
		processServiceCallback("SIGUSR2");
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

void SystemdServiceBackend::onStarted(bool success)
{
	if(success)
		sd_notify(false, "READY=1");
	else
		onStopped(EXIT_FAILURE);
}

void SystemdServiceBackend::onReloaded(bool success)
{
	if(!success) //TODO test
		sd_notify(false, "ERRNO=1");
	sd_notify(false, "READY=1");
}

void SystemdServiceBackend::onStopped(int exitCode)
{
	if(_watchdogTimer)
		_watchdogTimer->stop();
	qApp->exit(exitCode);
}

void SystemdServiceBackend::onPaused(bool success)
{
	if(success)
		kill(getpid(), SIGSTOP); //now actually stop
}

int SystemdServiceBackend::run(int &argc, char **argv, int flags)
{
	//prepare the app
	QCoreApplication app{argc, argv, flags};
	prepareWatchdog(); //do as early as possible
	if(!preStartService())
		return EXIT_FAILURE;

	connect(service(), QOverload<bool>::of(&Service::started),
			this, &SystemdServiceBackend::onStarted);
	connect(service(), QOverload<bool>::of(&Service::reloaded),
			this, &SystemdServiceBackend::onReloaded);
	connect(service(), QOverload<bool>::of(&Service::paused),
			this, &SystemdServiceBackend::onPaused,
			Qt::QueuedConnection);

	for(const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT, SIGUSR1, SIGUSR2})
		registerForSignal(signal);

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, StartCommand));
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

	int priority; // Informational
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
	default:
		Q_UNREACHABLE();
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
