#include "dummyservicebackend.h"
#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <csignal>
#endif

DummyServiceBackend::DummyServiceBackend(QObject *parent) :
	ServiceBackend(parent)
{}

int DummyServiceBackend::runService(QtService::Service *service, int &argc, char **argv, int flags)
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
	if(!service->preStart())
		return EXIT_FAILURE;

	connect(&app, &QCoreApplication::aboutToQuit,
			service, &QtService::Service::stop);

#ifdef Q_OS_WIN
	for(const auto signal : {CTRL_C_EVENT, CTRL_BREAK_EVENT}) {
#else
	for(const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT}) {
#endif
		registerForSignal(signal);
	}

	// start the eventloop
	QMetaObject::invokeMethod(service, "start", Qt::QueuedConnection);
	return app.exec();
}

void DummyServiceBackend::signalTriggered(int signal)
{
	switch(signal) {
#ifdef Q_OS_WIN
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
#else
	case SIGHUP:
		_service->processCommand(QtService::Service::ReloadCode);
		break;
	case SIGTSTP:
		_service->processCommand(QtService::Service::PauseCode);
		break;
	case SIGCONT:
		_service->processCommand(QtService::Service::ResumeCode);
		break;
	case SIGINT:
	case SIGTERM:
	case SIGQUIT:
#endif
		qApp->quit();
		break;
	default:
		ServiceBackend::signalTriggered(signal);
		break;
	}
}
