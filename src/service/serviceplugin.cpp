#include "serviceplugin.h"
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

QByteArrayList ServiceBackend::rawArguments(int argc, char **argv)
{
	Q_UNIMPLEMENTED();
	return {};
}

QHash<int, QByteArray> ServiceBackend::getActivatedSockets()
{
	return {};
}

void ServiceBackend::signalTriggered(int signal)
{
	qCWarning(logQtService) << "Unhandled signal:" << signal;
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

void ServiceBackend::startService(Service *service)
{
	service->start();
}

void ServiceBackend::stopService(Service *service)
{
	service->stop();
}

void ServiceBackend::processServiceCommand(Service *service, int code)
{
	service->processCommand(code);
}
