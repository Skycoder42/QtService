#include "systemdservicebackend.h"
#include <chrono>
#include <csignal>

#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>
#include <systemd/sd-daemon.h>

#include <QtCore/QTimer>
#include <QtService/private/logging_p.h>

using namespace QtService;

SystemdServiceBackend::SystemdServiceBackend(QObject *parent) :
	ServiceBackend(parent)
{}

int SystemdServiceBackend::runService(QtService::Service *service, int &argc, char **argv, int flags)
{
	qInstallMessageHandler(SystemdServiceBackend::systemdMessageHandler);
	QCoreApplication app(argc, argv, flags);

	//prepare the app
	prepareWatchdog(); //do as early as possible
	_service = service;
	if(!preStartService(_service))
		return EXIT_FAILURE;

	for(const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT})
		registerForSignal(signal);

	// start the eventloop
	QMetaObject::invokeMethod(this, "performStart", Qt::QueuedConnection);
	return app.exec();
}

void SystemdServiceBackend::quitService()
{
	connect(_service, &Service::stopped,
			qApp, &QCoreApplication::exit,
			Qt::UniqueConnection);
	sd_notify(false, "STOPPING=1");
	stopService(_service);
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
		sd_notify(false, "RELOADING=1");
		processServiceCommand(_service, Service::ReloadCode);
		sd_notify(false, "READY=1"); //TODO make async
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
	startService(_service);
	sd_notify(false, "READY=1");
}

void SystemdServiceBackend::sendWatchdog()
{
	sd_notify(false, "WATCHDOG=1");
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

		auto watchdogTimer = new QTimer(this);
		watchdogTimer->setTimerType(Qt::PreciseTimer);
		watchdogTimer->setInterval(mSecs);
		connect(watchdogTimer, &QTimer::timeout,
				this, &SystemdServiceBackend::sendWatchdog);
		watchdogTimer->start();
		qCDebug(logQtService) << "Enabled watchdog with an interval of" << mSecs.count() << "ms";
	}
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
