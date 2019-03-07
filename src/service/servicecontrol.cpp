#include "servicecontrol.h"
#include "servicecontrol_p.h"
#include "logging_p.h"
#include "service_p.h"

#include <chrono>

#include <QtCore/QTimer>

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

QString ServiceControl::serviceIdFromName(const QString &backend, const QString &name)
{
	return serviceIdFromName(backend, name, QCoreApplication::organizationDomain());
}

QString ServiceControl::serviceIdFromName(const QString &backend, const QString &name, const QString &domain)
{
	return ServicePrivate::idFromName(backend, name, domain);
}

ServiceControl *ServiceControl::create(const QString &backend, QString serviceId, QObject *parent)
{
	return ServicePrivate::createControl(backend, std::move(serviceId), parent);
}

ServiceControl *ServiceControl::create(const QString &backend, QString serviceId, QString serviceNameOverride, QObject *parent)
{
	auto control = ServicePrivate::createControl(backend, std::move(serviceId), parent);
	if(control)
		control->d->serviceName = std::move(serviceNameOverride);
	return control;
}

ServiceControl *ServiceControl::createFromName(const QString &backend, const QString &serviceName, QObject *parent)
{
	return ServicePrivate::createControl(backend, serviceIdFromName(backend, serviceName), parent);
}

ServiceControl *ServiceControl::createFromName(const QString &backend, const QString &serviceName, const QString &domain, QObject *parent)
{
	return ServicePrivate::createControl(backend, serviceIdFromName(backend, serviceName, domain), parent);
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

ServiceControl::BlockMode ServiceControl::blocking() const
{
	return BlockMode::Undetermined;
}

QString ServiceControl::error() const
{
	return d->error;
}

bool ServiceControl::isEnabled() const
{
	return true;
}

QVariant ServiceControl::callGenericCommand(const QByteArray &kind, const QVariantList &args)
{
	Q_UNUSED(args)
	setError(tr("Operation custom command for kind %1 is not implemented for backend %2")
			 .arg(QString::fromUtf8(kind), backend()));
	return {};
}

ServiceControl::Status ServiceControl::status() const
{
	setError(tr("Reading the service status is not implemented for backend %1")
			 .arg(backend()));
	return Status::Unknown;
}

bool ServiceControl::isAutostartEnabled() const
{
	setError(tr("Reading the autostart state is not implemented for backend %1")
			 .arg(backend()));
	return false;
}

QDir ServiceControl::runtimeDir() const
{
	return ServicePrivate::runtimeDir(realServiceName());
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

bool ServiceControl::restart()
{
	using namespace std::chrono_literals;

	// blocking services can simply call stop and start
	if(blocking() == BlockMode::Blocking) {
		auto ok = stop();
		if(ok)
			ok = start();
		return ok;
	// other types can still simulate a (non)-blocking start/stop, if the have status reports
	} else if(supportFlags().testFlag(SupportFlag::Status)) {
		if(!stop())
			return false;

		// check every second, until the service stopped or errored, then start again if successfully
		auto timer = new QTimer{this};
		connect(timer, &QTimer::timeout,
				this, [this, timer]() {
			switch(status()) {
			case Status::Stopped:
				timer->deleteLater();
				start(); //ignore result as error messages are set by start itself
				break;
			case Status::Errored:
				timer->deleteLater();
				break;
			// ignore all other cases
			default:
				break;
			}
		});
		timer->start(1s);
		return true;
	} else {
		setError(tr("Operation restart is not supported for non-blocking service controls without status information"));
		return false;
	}
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

bool ServiceControl::setBlocking(bool blocking)
{
	Q_UNUSED(blocking)
	return false;
}

void ServiceControl::clearError()
{
	setError(QString{});
}

bool ServiceControl::setEnabled(bool enabled)
{
	Q_UNUSED(enabled);
	return false;
}

QString ServiceControl::serviceName() const
{
	return serviceId();
}

QString ServiceControl::realServiceName() const
{
	return d->serviceName.isNull() ? serviceName() : d->serviceName;
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
