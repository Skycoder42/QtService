#include "launchdserviceplugin.h"
#include "launchdservicebackend.h"
#include "launchdservicecontrol.h"

Q_LOGGING_CATEGORY(logQtService, "qtservice.servicebackends.launchd"); //TODO, QtInfoMsg);

LaunchdServicePlugin::LaunchdServicePlugin(QObject *parent) :
	QObject{parent}
{}

QString LaunchdServicePlugin::currentServiceId() const
{
	return QCoreApplication::organizationDomain() + QLatin1Char('.') + QCoreApplication::applicationName();
}

QtService::ServiceBackend *LaunchdServicePlugin::createServiceBackend(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("launchd"))
		return new LaunchdServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *LaunchdServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	if(backend == QStringLiteral("launchd"))
		return new LaunchdServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
