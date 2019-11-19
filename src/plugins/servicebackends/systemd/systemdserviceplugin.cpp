#include "systemdserviceplugin.h"
#include "systemdservicebackend.h"
#include "systemdservicecontrol.h"
using namespace QtService;

SystemdServicePlugin::SystemdServicePlugin(QObject *parent) :
	QObject(parent)
{}

QString SystemdServicePlugin::findServiceId(const QString &backend, const QString &serviceName, const QString &domain) const
{
	Q_UNUSED(domain)
	if (backend == QStringLiteral("systemd"))
		return serviceName + QStringLiteral(".service");
	else
		return {};
}

ServiceBackend *SystemdServicePlugin::createServiceBackend(const QString &backend, Service *service)
{
	if (backend == QStringLiteral("systemd"))
		return new SystemdServiceBackend{service};
	else
		return nullptr;
}

ServiceControl *SystemdServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	if (backend == QStringLiteral("systemd"))
		return new SystemdServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
