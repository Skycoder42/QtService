#include "servicecontrol.h"
#include "servicecontrol_p.h"
#include "logging_p.h"
#include "service_p.h"
using namespace QtService;

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

void ServiceControl::start()
{
	qCWarning(logQtService) << "Operation start is not implemented with backend" << backend();
}

void ServiceControl::stop()
{
	qCWarning(logQtService) << "Operation stop is not implemented with backend" << backend();
}

void ServiceControl::pause()
{
	qCWarning(logQtService) << "Operation pause is not implemented with backend" << backend();
}

void ServiceControl::resume()
{
	qCWarning(logQtService) << "Operation resume is not implemented with backend" << backend();
}

void ServiceControl::reload()
{
	qCWarning(logQtService) << "Operation reload is not implemented with backend" << backend();
}

void ServiceControl::enable()
{
	qCWarning(logQtService) << "Operation enable is not implemented with backend" << backend();
}

void ServiceControl::disable()
{
	qCWarning(logQtService) << "Operation disable is not implemented with backend" << backend();
}

void ServiceControl::setEnabled(bool enabled)
{
	if(enabled)
		enable();
	else
		disable();
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
