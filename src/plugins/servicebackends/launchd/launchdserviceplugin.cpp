#include "launchdserviceplugin.h"
#include "launchdservicebackend.h"

LaunchdServicePlugin::LaunchdServicePlugin(QObject *parent) :
	QObject{parent}
{}

QtService::ServiceBackend *LaunchdServicePlugin::createInstance(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("standard"))
		return new LaunchdServiceBackend{service};
	else
		return nullptr;
}
