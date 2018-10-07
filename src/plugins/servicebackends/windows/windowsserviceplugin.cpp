#include "windowsserviceplugin.h"
#include "windowsservicebackend.h"
#include "windowsservicecontrol.h"

Q_LOGGING_CATEGORY(logQtService, "qtservice.servicebackends.windows", QtInfoMsg);

WindowsServicePlugin::WindowsServicePlugin(QObject *parent) :
	QObject(parent)
{}

QString WindowsServicePlugin::currentServiceId() const
{
	return QCoreApplication::applicationName();
}

QtService::ServiceBackend *WindowsServicePlugin::createServiceBackend(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("windows"))
		return new WindowsServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *WindowsServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	auto parts = detectNamedService(serviceId);
	if(!parts.first.isEmpty())
		serviceId = parts.first;

	if(backend == QStringLiteral("windows"))
		return new WindowsServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
