#include "launchdservicebackend.h"
#include <csignal>
#include <unistd.h>
#include <launch.h>
using namespace QtService;

LaunchdServiceBackend::LaunchdServiceBackend(Service *service) :
	ServiceBackend{service}
{}

int LaunchdServiceBackend::runService(int &argc, char **argv, int flags)
{
	qSetMessagePattern(QStringLiteral("[%{time} "
									  "%{if-debug}\033[32mDebug\033[0m]    %{endif}"
									  "%{if-info}\033[36mInfo\033[0m]     %{endif}"
									  "%{if-warning}\033[33mWarning\033[0m]  %{endif}"
									  "%{if-critical}\033[31mCritical\033[0m] %{endif}"
									  "%{if-fatal}\033[35mFatal\033[0m]    %{endif}"
									  "%{if-category}%{category}: %{endif}"
									  "%{message}"));

	QCoreApplication app(argc, argv, flags);
	if(!preStartService())
		return EXIT_FAILURE;

	connect(service(), &Service::paused,
			this, &LaunchdServiceBackend::onPaused,
			Qt::QueuedConnection);

	for(const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT, SIGUSR1, SIGUSR2})
		registerForSignal(signal);

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, StartCommand));
	return app.exec();
}

void LaunchdServiceBackend::quitService()
{
	connect(service(), &Service::stopped,
			qApp, &QCoreApplication::exit,
			Qt::UniqueConnection);
	processServiceCommand(StopCommand);
}

void LaunchdServiceBackend::reloadService()
{
	processServiceCommand(ReloadCommand);
}

QHash<int, QByteArray> LaunchdServiceBackend::getActivatedSockets()
{
//	int *fds;
//	size_t cnt;

//	int err = launch_activate_socket(*handleName, &fds, &cnt);
}

void LaunchdServiceBackend::signalTriggered(int signal)
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

void LaunchdServiceBackend::onPaused()
{
	kill(getpid(), SIGSTOP); //now actually stop
}
