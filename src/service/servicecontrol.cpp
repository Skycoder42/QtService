#include "servicecontrol.h"
#include "servicecontrol_p.h"
#include "logging_p.h"
#include "service_p.h"
using namespace QtService;

QStringList ServiceControl::listBackends()
{
	return ServicePrivate::listBackends();
}

QString ServiceControl::likelyBackend()
{
#if defined(Q_OS_ANDROID)
	auto backend = QStringLiteral("android");
#elif defined(Q_OS_LINUX)
	auto backend = QStringLiteral("systemd");
#elif defined(Q_OS_MACOS)
	auto backend = QStringLiteral("launchd");
#elif defined(Q_OS_WIN32)
	auto backend = QStringLiteral("windows");
#else
	QString backend;
#endif
	if(ServicePrivate::listBackends().contains(backend))
		return backend;
	else
		return QStringLiteral("standard");
}

ServiceControl *ServiceControl::create(const QString &backend, QString serviceId, QObject *parent)
{
	auto control = ServicePrivate::createControl(backend, std::move(serviceId), parent);
	// set the correct default value
	if(control)
		control->d->blocking = control->supportFlags().testFlag(SupportsBlocking);
	return control;
}

ServiceControl::ServiceControl(QString &&serviceId, QObject *parent) :
	QObject{parent},
	d{new ServiceControlPrivate{std::move(serviceId)}}
{}

ServiceControl::~ServiceControl() = default;

QString ServiceControl::serviceId() const
{
	return d->serviceId;
}

bool ServiceControl::isBlocking() const
{
	return d->blocking;
}

QString ServiceControl::error() const
{
	return d->error;
}

QVariant ServiceControl::callGenericCommand(const QByteArray &kind, const QVariantList &args)
{
	Q_UNUSED(args)
	setError(tr("Operation custom command for kind %1 is not implemented for backend %2")
			 .arg(QString::fromUtf8(kind), backend()));
	return {};
}

ServiceControl::ServiceStatus ServiceControl::status() const
{
	setError(tr("Reading the service status is not implemented for backend %1")
			 .arg(backend()));
	return ServiceStatusUnknown;
}

bool ServiceControl::isAutostartEnabled() const
{
	setError(tr("Reading the autostart state is not implemented for backend %1")
			 .arg(backend()));
	return false;
}

QDir ServiceControl::runtimeDir() const
{
	return ServicePrivate::runtimeDir(serviceName());
}

bool ServiceControl::start()
{
	setError(tr("Operation start is not implemented for backend %1")
			 .arg(backend()));
	return false;
}

bool ServiceControl::stop()
{
	setError(tr("Operation stop is not implemented for backend %1")
			 .arg(backend()));
	return false;
}

bool ServiceControl::pause()
{
	setError(tr("Operation pause is not implemented for backend %1")
			 .arg(backend()));
	return false;
}

bool ServiceControl::resume()
{
	setError(tr("Operation resume is not implemented for backend %1")
			 .arg(backend()));
	return false;
}

bool ServiceControl::reload()
{
	setError(tr("Operation reload is not implemented for backend %1")
			 .arg(backend()));
	return false;
}

bool ServiceControl::enableAutostart()
{
	setError(tr("Operation enable autostart is not implemented for backend %1")
			 .arg(backend()));
	return false;
}

bool ServiceControl::disableAutostart()
{
	setError(tr("Operation disable autostart is not implemented for backend %1")
			 .arg(backend()));
	return false;
}

void ServiceControl::setBlocking(bool blocking)
{
	if (d->blocking == blocking)
		return;

	if(blocking && !supportFlags().testFlag(SupportsBlocking))
		return;
	if(!blocking && !supportFlags().testFlag(SupportsNonBlocking))
		return;

	d->blocking = blocking;
	emit blockingChanged(d->blocking, {});
}

void ServiceControl::clearError()
{
	setError(QString{});
}

QString ServiceControl::serviceName() const
{
	return serviceId();
}

void ServiceControl::setError(QString error) const
{
	if (d->error == error)
		return;

	d->error = std::move(error);
	emit const_cast<ServiceControl*>(this)->errorChanged(d->error, {});
}

// ------------- Private Implementation -------------

ServiceControlPrivate::ServiceControlPrivate(QString &&serviceId) :
	serviceId{std::move(serviceId)}
{}
