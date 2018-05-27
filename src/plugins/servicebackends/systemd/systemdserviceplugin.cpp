#include "systemdserviceplugin.h"
#include "systemdservicebackend.h"
using namespace QtService;

SystemdServicePlugin::SystemdServicePlugin(QObject *parent) :
	QObject(parent)
{}

ServiceBackend *SystemdServicePlugin::createInstance(const QString &provider, Service *service)
{
	if(provider == QStringLiteral("systemd"))
		return new SystemdServiceBackend{service};
	else
		return nullptr;
}
