#include "windowsserviceplugin.h"
#include "windowsservicebackend.h"
#include "windowsservicecontrol.h"

Q_LOGGING_CATEGORY(logQtService, "qtservice.servicebackends.windows", QtInfoMsg);

WindowsServicePlugin::WindowsServicePlugin(QObject *parent) :
	QObject(parent)
{}

QString WindowsServicePlugin::findServiceId(const QString &backend, const QString &serviceName, const QString &domain) const
{
	Q_UNUSED(domain)
	if (backend == QStringLiteral("windows"))
		return serviceName;
	else
		return {};
}

QtService::ServiceBackend *WindowsServicePlugin::createServiceBackend(const QString &backend, QtService::Service *service)
{
	if (backend == QStringLiteral("windows"))
		return new WindowsServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *WindowsServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	if (backend == QStringLiteral("windows"))
		return new WindowsServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
