#include "systemdserviceplugin.h"
#include "systemdservicebackend.h"
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

}
