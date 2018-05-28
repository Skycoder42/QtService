#include "servicecontrol.h"
#include "servicecontrol_p.h"
#include "logging_p.h"
#include "service_p.h"
using namespace QtService;

QStringList ServiceControl::listBackends()
{
	return ServicePrivate::listBackends();
}

ServiceControl *ServiceControl::create(const QString &backend, QString serviceId, QObject *parent)
{
	return ServicePrivate::createControl(backend, std::move(serviceId), parent);
}

ServiceControl::ServiceControl(QString &&serviceId, QObject *parent) :
	QObject{parent},
	d{new ServiceControlPrivate{std::move(serviceId)}}
{}

QString ServiceControl::serviceId() const
{
	return d->serviceId;
}

bool ServiceControl::isBlocking() const
{
	return d->blocking;
}

ServiceControl::ServiceStatus ServiceControl::status() const
{
	return ServiceStatusUnknown;
}

bool ServiceControl::isEnabled() const
{
	return false;
}

QVariant ServiceControl::callGenericCommand(const QByteArray &kind, const QVariantList &args)
{
	Q_UNUSED(kind)
	Q_UNUSED(args)
	qCWarning(logQtService) << "Operation custom command is not implemented with backend" << backend();
	return {};
}

bool ServiceControl::start()
{
	qCWarning(logQtService) << "Operation start is not implemented with backend" << backend();
	return false;
}

bool ServiceControl::stop()
{
	qCWarning(logQtService) << "Operation stop is not implemented with backend" << backend();
	return false;
}

bool ServiceControl::pause()
{
	qCWarning(logQtService) << "Operation pause is not implemented with backend" << backend();
	return false;
}

bool ServiceControl::resume()
{
	qCWarning(logQtService) << "Operation resume is not implemented with backend" << backend();
	return false;
}

bool ServiceControl::reload()
{
	qCWarning(logQtService) << "Operation reload is not implemented with backend" << backend();
	return false;
}

bool ServiceControl::enable()
{
	qCWarning(logQtService) << "Operation enable is not implemented with backend" << backend();
	return false;
}

bool ServiceControl::disable()
{
	qCWarning(logQtService) << "Operation disable is not implemented with backend" << backend();
	return false;
}

void ServiceControl::setBlocking(bool blocking)
{
	if (d->blocking == blocking)
		return;

	d->blocking = blocking;
	emit blockingChanged(d->blocking, {});
}

bool ServiceControl::setEnabled(bool enabled)
{
	if(enabled)
		return enable();
	else
		return disable();
}

QString ServiceControl::serviceName() const
{
	return serviceId();
}

QDir ServiceControl::runtimeDir() const
{
	return ServicePrivate::runtimeDir(serviceName());
}

ServiceControl::~ServiceControl() = default;

ServiceControlPrivate::ServiceControlPrivate(QString &&serviceId) :
	serviceId{std::move(serviceId)}
{}
