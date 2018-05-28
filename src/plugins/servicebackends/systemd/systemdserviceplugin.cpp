#include "systemdserviceplugin.h"
#include "systemdservicebackend.h"
#include "systemdservicecontrol.h"
using namespace QtService;

SystemdServicePlugin::SystemdServicePlugin(QObject *parent) :
	QObject(parent)
{}

ServiceBackend *SystemdServicePlugin::createServiceBackend(const QString &provider, Service *service)
{
	if(provider == QStringLiteral("systemd"))
		return new SystemdServiceBackend{service};
	else
		return nullptr;
}

ServiceControl *SystemdServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	if(backend == QStringLiteral("systemd"))
		return new SystemdServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
