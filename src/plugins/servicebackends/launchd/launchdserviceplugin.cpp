#include "launchdserviceplugin.h"
#include "launchdservicebackend.h"
#include "launchdservicecontrol.h"

Q_LOGGING_CATEGORY(logQtService, "qtservice.servicebackends.launchd", QtInfoMsg);

LaunchdServicePlugin::LaunchdServicePlugin(QObject *parent) :
	QObject{parent}
{}

QString LaunchdServicePlugin::findServiceId(const QString &backend, const QString &serviceName, const QString &domain) const
{
	if (backend == QStringLiteral("launchd"))
		return domain.isEmpty() ? serviceName : domain + QLatin1Char('.') + serviceName;
	else
		return {};
}

QtService::ServiceBackend *LaunchdServicePlugin::createServiceBackend(const QString &backend, QtService::Service *service)
{
	if (backend == QStringLiteral("launchd"))
		return new LaunchdServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *LaunchdServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	if (backend == QStringLiteral("launchd"))
		return new LaunchdServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
