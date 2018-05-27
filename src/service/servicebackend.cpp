#include "servicebackend.h"
#include "servicebackend_p.h"
#include "service_p.h"
#include <QtService/private/logging_p.h>
#include <QCtrlSignals>
using namespace QtService;

#define EXTEND(a, x, ...) [a](auto... args) { \
	(a->*(x))(args..., __VA_ARGS__); \
}
#define PSIG(x) EXTEND(this, x, QPrivateSignal{})

ServiceBackend::ServiceBackend(Service *service) :
	QObject{service},
	d{new ServiceBackendPrivate{service}}
{
	connect(d->service, &Service::resumed,
			this, &ServiceBackend::onSvcResumed);
	connect(d->service, &Service::paused,
			this, &ServiceBackend::onSvcPaused);
}

QList<int> ServiceBackend::getActivatedSockets(const QByteArray &name)
{
	Q_UNUSED(name)
	return {};
}

ServiceBackend::~ServiceBackend() = default;

void ServiceBackend::signalTriggered(int signal)
{
	qCWarning(logQtService) << "Unhandled signal:" << signal;
}

void ServiceBackend::processServiceCommand(ServiceCommand code)
{
	//TODO add support for "pending" commands
	// => ignore new commands if an old one of the same time has not yet been processed???
	switch(code) {
	case StartCommand:
		if(d->service->onStart() == Service::Synchronous)
			emit d->service->started();
		break;
	case StopCommand:
	{
		auto exitCode = EXIT_SUCCESS;
		if(d->service->onStop(exitCode) == Service::Synchronous)
			emit d->service->stopped(exitCode);
		break;
	}
	case ReloadCommand:
		if(d->service->onReload() == Service::Synchronous)
			emit d->service->reloaded();
		break;
	case PauseCommand:
		if(!d->service->d->wasPaused) {
			if(d->service->onPause() == Service::Synchronous)
				emit d->service->paused();
		}
		break;
	case ResumeCommand:
		if(d->service->d->wasPaused) {
			if(d->service->onResume() == Service::Synchronous)
				emit d->service->resumed();
		}
		break;
	}
}

QVariant ServiceBackend::processServiceCallbackImpl(const QByteArray &kind, const QVariantList &args)
{
	return d->service->onCallback(kind, args);
}

Service *ServiceBackend::service() const
{
	return d->service;
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

bool ServiceBackend::preStartService()
{
	return d->service->preStart();
}

void ServiceBackend::onSvcResumed()
{
	d->service->d->wasPaused = false;
}

void ServiceBackend::onSvcPaused()
{
	d->service->d->wasPaused = true;
}

// ------------- Private implementation -------------

ServiceBackendPrivate::ServiceBackendPrivate(Service *service) :
	service{service}
{}
