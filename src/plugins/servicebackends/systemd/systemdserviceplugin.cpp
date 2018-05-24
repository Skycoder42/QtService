#include "systemdserviceplugin.h"
#include "systemdservicebackend.h"
using namespace QtService;

SystemdServicePlugin::SystemdServicePlugin(QObject *parent) :
	QObject(parent)
{}

ServiceBackend *SystemdServicePlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == QStringLiteral("systemd"))
		return new SystemdServiceBackend(parent);
	else
		return nullptr;
}
