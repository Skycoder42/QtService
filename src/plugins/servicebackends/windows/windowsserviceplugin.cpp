#include "windowsserviceplugin.h"
#include "windowsservicebackend.h"

WindowsServicePlugin::WindowsServicePlugin(QObject *parent) :
	QObject(parent)
{}

QtService::ServiceBackend *WindowsServicePlugin::createServiceBackend(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("windows"))
		return new WindowsServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *WindowsServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{

}
