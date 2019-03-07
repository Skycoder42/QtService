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
	connect(d->service, &Service::started,
			this, &ServiceBackend::onSvcStarted);
	connect(d->service, &Service::stopped,
			this, &ServiceBackend::onSvcStopped);
	connect(d->service, &Service::reloaded,
			this, &ServiceBackend::onSvcReloaded);
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
	if(d->operating && code != ServiceCommand::Stop) { //always allow stopping
		qCWarning(logQtService) << "Ignoring command of type" << code << "as another command is currently beeing processed";
		return;
	}

	d->operating = true;
	switch(code) {
	case ServiceCommand::Start:
		switch(d->service->onStart()) {
		case Service::CommandResult::Completed:
			emit d->service->started(true);
			break;
		case Service::CommandResult::Failed:
			emit d->service->started(false);
			break;
		case Service::CommandResult::Pending:
			break;
		case Service::CommandResult::Exit:
			emit d->service->started(true);
			quitService();
			break;
		default: //all other cases should never happen
			Q_UNREACHABLE();
			break;
		}
		break;
	case ServiceCommand::Stop:
	{
		auto exitCode = EXIT_SUCCESS;
		switch(d->service->onStop(exitCode)) {
		case Service::CommandResult::Completed:
			emit d->service->stopped(exitCode);
			break;
		case Service::CommandResult::Failed:
			qCWarning(logQtService) << "The stop-operation should never fail. The result is ignored and the service will stop anyways";
			emit d->service->stopped(exitCode == EXIT_SUCCESS ? EXIT_FAILURE : exitCode);
			break;
		case Service::CommandResult::Pending:
			break;
		default: //all other cases should never happen
			Q_UNREACHABLE();
			break;
		}
		break;
	}
	case ServiceCommand::Reload:
		switch(d->service->onReload()) {
		case Service::CommandResult::Completed:
			emit d->service->reloaded(true);
			break;
		case Service::CommandResult::Failed:
			emit d->service->reloaded(false);
			break;
		case Service::CommandResult::Pending:
			break;
		default: //all other cases should never happen
			Q_UNREACHABLE();
			break;
		}
		break;
	case ServiceCommand::Pause:
		if(!d->service->d->wasPaused) {
			switch(d->service->onPause()) {
			case Service::CommandResult::Completed:
				emit d->service->paused(true);
				break;
			case Service::CommandResult::Failed:
				emit d->service->paused(false);
				break;
			case Service::CommandResult::Pending:
				break;
			default: //all other cases should never happen
				Q_UNREACHABLE();
				break;
			}
		}
		break;
	case ServiceCommand::Resume:
		if(d->service->d->wasPaused) {
			switch(d->service->onResume()) {
			case Service::CommandResult::Completed:
				emit d->service->resumed(true);
				break;
			case Service::CommandResult::Failed:
				emit d->service->resumed(false);
				break;
			case Service::CommandResult::Pending:
				break;
			default: //all other cases should never happen
				Q_UNREACHABLE();
				break;
			}
		}
		break;
	default:
		Q_UNREACHABLE();
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
	d->operating = false;
	if(success) {
		d->service->d->isRunning = true;
		d->service->d->startTerminals();
	} // proper stopping is handled by the backends
}

void ServiceBackend::onSvcStopped()
{
	d->operating = false;
	d->service->d->stopTerminals();
	d->service->d->isRunning = false;
}

void ServiceBackend::onSvcReloaded(bool success)
{
	d->operating = false;
	Q_UNUSED(success)
}

void ServiceBackend::onSvcResumed(bool success)
{
	d->operating = false;
	if(success)
		d->service->d->wasPaused = false;
}

void ServiceBackend::onSvcPaused(bool success)
{
	d->operating = false;
	if(success)
		d->service->d->wasPaused = true;
}

// ------------- Private implementation -------------

ServiceBackendPrivate::ServiceBackendPrivate(Service *service) :
	service{service}
{}
