#include "launchdservicebackend.h"
#include <QtService/private/logging_p.h>
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

QList<int> LaunchdServiceBackend::getActivatedSockets(const QByteArray &name)
{
	auto mName = name.isNull() ? QByteArrayLiteral("Listeners") : name;
	if(!_socketCache.contains(mName)) {
		int *fds = nullptr;
		size_t cnt = 0;
		int err = launch_activate_socket(mName.constData(), &fds, &cnt);
		if(err != 0)
			qCWarning(logQtService) << "Failed to get sockets with error:" << strerror(err);
		else {
			_socketCache.reserve(_socketCache.size() + static_cast<int>(cnt));
			for(size_t i = 0; i < cnt; i++)
				_socketCache.insert(mName, fds[i]);
		}
		if(fds)
			free(fds);
	}

	return _socketCache.values(mName);
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
