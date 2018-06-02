#include "standardservicebackend.h"
#include "standardserviceplugin.h"
#include <QtCore/QLockFile>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <csignal>
#include <unistd.h>
#endif
using namespace QtService;

StandardServiceBackend::StandardServiceBackend(Service *service) :
	ServiceBackend{service}
{}

int StandardServiceBackend::runService(int &argc, char **argv, int flags)
{
	//setup logging
#ifdef Q_OS_WIN
	qSetMessagePattern(QStringLiteral("[%{time} "
									  "%{if-debug}Debug]    %{endif}"
									  "%{if-info}Info]     %{endif}"
									  "%{if-warning}Warning]  %{endif}"
									  "%{if-critical}Critical] %{endif}"
									  "%{if-fatal}Fatal]    %{endif}"
									  "%{if-category}%{category}: %{endif}"
									  "%{message}"));
#else
	qSetMessagePattern(QStringLiteral("[%{time} "
									  "%{if-debug}\033[32mDebug\033[0m]    %{endif}"
									  "%{if-info}\033[36mInfo\033[0m]     %{endif}"
									  "%{if-warning}\033[33mWarning\033[0m]  %{endif}"
									  "%{if-critical}\033[31mCritical\033[0m] %{endif}"
									  "%{if-fatal}\033[35mFatal\033[0m]    %{endif}"
									  "%{if-category}%{category}: %{endif}"
									  "%{message}"));
#endif

	QCoreApplication app(argc, argv, flags);
	if(!preStartService())
		return EXIT_FAILURE;

	// create lock
	QLockFile lock{service()->runtimeDir().absoluteFilePath(QStringLiteral("qstandard.lock"))};
	lock.setStaleLockTime(std::numeric_limits<int>::max()); //disable stale locks
	if(!lock.tryLock(5000)) {
		qCCritical(logQtService) << "Failed to create service lock in"
								 << service()->runtimeDir().absolutePath()
								 << "with error code:" << lock.error();
		qint64 pid = 0;
		QString hostname, appname;
		if(lock.getLockInfo(&pid, &hostname, &appname)) {
			qCCritical(logQtService).noquote() << "Lock information:"
											   << "\n\tPID:" << pid
											   << "\n\tHostname:" << hostname
											   << "\n\tAppname:" << appname;
		}
		return EXIT_FAILURE;
	}

	//ensure unlocking always works
	connect(qApp, &QCoreApplication::aboutToQuit,
			this, [&]() {
		lock.unlock();
	});
	connect(service(), &Service::paused,
			this, &StandardServiceBackend::onPaused,
			Qt::QueuedConnection);

#ifdef Q_OS_WIN
	for(const auto signal : {CTRL_C_EVENT, CTRL_BREAK_EVENT}) {
#else
	for(const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT, SIGUSR1, SIGUSR2}) {
#endif
		registerForSignal(signal);
	}

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, StartCommand));
	return app.exec();
}

void StandardServiceBackend::quitService()
{
	connect(service(), &Service::stopped,
			qApp, &QCoreApplication::exit,
			Qt::UniqueConnection);
	processServiceCommand(StopCommand);
}

void StandardServiceBackend::reloadService()
{
	processServiceCommand(ReloadCommand);
}

void StandardServiceBackend::signalTriggered(int signal)
{
	switch(signal) {
#ifdef Q_OS_WIN
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
		quitService();
		break;
#else
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
#endif
	default:
		ServiceBackend::signalTriggered(signal);
		break;
	}
}

void StandardServiceBackend::onPaused()
{
#ifdef Q_OS_UNIX
	kill(getpid(), SIGSTOP); //now actually stop
#endif
}

