#include "serviceplugin.h"
#include "service_p.h"
#include <QtService/private/logging_p.h>
#include <QCtrlSignals>
using namespace QtService;

#define EXTEND(a, x, ...) [a](auto... args) { \
	(a->*(x))(args..., __VA_ARGS__); \
}
#define PSIG(x) EXTEND(this, x, QPrivateSignal{})

ServiceBackend::ServiceBackend(QObject *parent) :
	QObject{parent}
{}

QHash<int, QByteArray> ServiceBackend::getActivatedSockets()
{
	return {};
}

void ServiceBackend::signalTriggered(int signal)
{
	qCWarning(logQtService) << "Unhandled signal:" << signal;
}

void ServiceBackend::processServiceCommand(Service *service, ServiceCommand code)
{
	//TODO add support for "pending" commands
	// => ignore new commands if an old one of the same time has not yet been processed???
	switch(code) {
	case StartCommand:
		if(service->onStart() == Service::Synchronous)
			emit service->started();
		break;
	case StopCommand:
	{
		auto exitCode = EXIT_SUCCESS;
		if(service->onStop(exitCode) == Service::Synchronous)
			emit service->stopped(exitCode);
		break;
	}
	case ReloadCommand:
		if(service->onReload() == Service::Synchronous)
			emit service->reloaded();
		break;
	case PauseCommand:
		service->onPause();
		service->d->wasPaused = true;
		break;
	case ResumeCommand:
		if(service->d->wasPaused) {
			service->onResume();
			service->d->wasPaused = false;
		}
		break;
	}
}

QVariant ServiceBackend::processServiceCallbackImpl(Service *service, const QByteArray &kind, const QVariantList &args)
{
	return service->onCallback(kind, args);
}

bool ServiceBackend::registerForSignal(int signal)
{
	auto handler = QCtrlSignalHandler::instance();
	connect(handler, &QCtrlSignalHandler::ctrlSignal,
			this, &ServiceBackend::signalTriggered,
			Qt::UniqueConnection);
	return handler->registerForSignal(signal);
}

bool ServiceBackend::unregisterFromSignal(int signal)
{
	return QCtrlSignalHandler::instance()->unregisterFromSignal(signal);
}

bool ServiceBackend::preStartService(Service *service)
{
	return service->preStart();
}
