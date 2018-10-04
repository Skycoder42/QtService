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
	connect(d->service, QOverload<bool>::of(&Service::started),
			this, &ServiceBackend::onSvcStarted);
	connect(d->service, &Service::stopped,
			this, &ServiceBackend::onSvcStopped);
	connect(d->service, QOverload<bool>::of(&Service::resumed),
			this, &ServiceBackend::onSvcResumed);
	connect(d->service, QOverload<bool>::of(&Service::paused),
			this, &ServiceBackend::onSvcPaused);

	//MAJOR compat remove (and remove overload on previous definitions)
	QT_WARNING_PUSH
	QT_WARNING_DISABLE_DEPRECATED
	connect(d->service, QOverload<>::of(&Service::started),
			this, std::bind(QOverload<bool>::of(&ServiceBackend::onSvcStarted), this, true));
	connect(d->service, QOverload<>::of(&Service::resumed),
			this, std::bind(QOverload<bool>::of(&ServiceBackend::onSvcResumed), this, true));
	connect(d->service, QOverload<>::of(&Service::paused),
			this, std::bind(QOverload<bool>::of(&ServiceBackend::onSvcPaused), this, true));
	QT_WARNING_POP
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
	// => ignore new commands if an old one of the same kind has not yet been processed???
	switch(code) {
	case StartCommand:
		switch(d->service->onStart()) {
		case Service::OperationCompleted:
			emit d->service->started(true);
			break;
		case Service::OperationFailed:
			emit d->service->started(false);
			break;
		case Service::OperationPending:
			break;
		case Service::OperationExit:
			Q_UNIMPLEMENTED(); //TODO implement
			break;
		default: //all other cases should never happen
			Q_UNREACHABLE();
		}
		break;
	case StopCommand:
	{
		auto exitCode = EXIT_SUCCESS;
		switch(d->service->onStop(exitCode)) {
		case Service::OperationCompleted:
			emit d->service->stopped(exitCode);
			break;
		case Service::OperationFailed:
			qWarning() << "The stop-operation should never fail. The result is ignored and the service will stop anyways";
			emit d->service->stopped(exitCode == EXIT_SUCCESS ? EXIT_FAILURE : exitCode);
			break;
		case Service::OperationPending:
			break;
		default: //all other cases should never happen
			Q_UNREACHABLE();
		}
		break;
	}
	case ReloadCommand:
		switch(d->service->onReload()) {
		case Service::OperationCompleted:
			emit d->service->reloaded(true);
			break;
		case Service::OperationFailed:
			emit d->service->reloaded(false);
			break;
		case Service::OperationPending:
			break;
		default: //all other cases should never happen
			Q_UNREACHABLE();
		}
		break;
	case PauseCommand:
		if(!d->service->d->wasPaused) {
			switch(d->service->onPause()) {
			case Service::OperationCompleted:
				emit d->service->paused(true);
				break;
			case Service::OperationFailed:
				emit d->service->paused(false);
				break;
			case Service::OperationPending:
				break;
			default: //all other cases should never happen
				Q_UNREACHABLE();
			}
		}
		break;
	case ResumeCommand:
		if(d->service->d->wasPaused) {
			switch(d->service->onResume()) {
			case Service::OperationCompleted:
				emit d->service->resumed(true);
				break;
			case Service::OperationFailed:
				emit d->service->resumed(false);
				break;
			case Service::OperationPending:
				break;
			default: //all other cases should never happen
				Q_UNREACHABLE();
			}
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

void ServiceBackend::onSvcStarted(bool success)
{
	if(success) {
		d->service->d->isRunning = true;
		d->service->d->startTerminals();
	} // proper stopping is handled by the backends
}

void ServiceBackend::onSvcStopped()
{
	d->service->d->stopTerminals();
	d->service->d->isRunning = false;
}

void ServiceBackend::onSvcResumed(bool success)
{
	if(success)
		d->service->d->wasPaused = false;
}

void ServiceBackend::onSvcPaused(bool success)
{
	if(success)
		d->service->d->wasPaused = true;
}

// ------------- Private implementation -------------

ServiceBackendPrivate::ServiceBackendPrivate(Service *service) :
	service{service}
{}
