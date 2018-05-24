#include "serviceplugin.h"
#include <QCtrlSignals>
#include <QDebug>
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

void ServiceBackend::signalTriggered(int signal)
{
	qWarning() << "Unhandled signal:" << signal;
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
