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
	QCoreApplication app(argc, argv, flags);

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

	//prepare the app
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
	QMetaObject::invokeMethod(this, "startService", Qt::QueuedConnection,
							  Q_ARG(QtService::Service*, _service));
	return app.exec();
}

void StandardServiceBackend::quitService()
{
	connect(_service, &Service::stopped,
			qApp, &QCoreApplication::exit,
			Qt::UniqueConnection);
	stopService(_service);
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
	case SIGUSR1:
	case SIGUSR2:
		processServiceCommand(_service, signal); //TODO translate?
		break;
	case SIGHUP:
		processServiceCommand(_service, Service::ReloadCode);
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
#endif
	default:
		ServiceBackend::signalTriggered(signal);
		break;
	}
}
