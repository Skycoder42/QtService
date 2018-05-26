#include "standardservicebackend.h"
#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <csignal>
#endif
using namespace QtService;

StandardServiceBackend::StandardServiceBackend(QObject *parent) :
	ServiceBackend(parent)
{}

int StandardServiceBackend::runService(Service *service, int &argc, char **argv, int flags)
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
	_service = service;
	if(!preStartService(_service))
		return EXIT_FAILURE;

#ifdef Q_OS_WIN
	for(const auto signal : {CTRL_C_EVENT, CTRL_BREAK_EVENT}) {
#else
	for(const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT, SIGUSR1, SIGUSR2}) {
#endif
		registerForSignal(signal);
	}

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::Service*, _service),
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, StartCommand));
	return app.exec();
}

void StandardServiceBackend::quitService()
{
	connect(_service, &Service::stopped,
			qApp, &QCoreApplication::exit,
			Qt::UniqueConnection);
	processServiceCommand(_service, StopCommand);
}

void StandardServiceBackend::reloadService()
{
	processServiceCommand(_service, ReloadCommand);
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
		processServiceCommand(_service, PauseCommand);
		break;
	case SIGCONT:
		processServiceCommand(_service, ResumeCommand);
		break;
	case SIGUSR1:
		processServiceCallback(_service, "SIGUSR1");
		break;
	case SIGUSR2:
		processServiceCallback(_service, "SIGUSR2");
		break;
#endif
	default:
		ServiceBackend::signalTriggered(signal);
		break;
	}
}
