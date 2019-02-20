#include "androidserviceplugin.h"
#include "androidservicebackend.h"
#include "androidservicecontrol.h"
#include <QtCore/QCoreApplication>

Q_LOGGING_CATEGORY(logQtService, "qtservice.servicebackends.android", QtInfoMsg);

AndroidServicePlugin::AndroidServicePlugin(QObject *parent) :
	QObject{parent}
{}

QString AndroidServicePlugin::currentServiceId() const
{
	return QCoreApplication::organizationDomain() + QLatin1Char('.') + QCoreApplication::applicationName();
}

QtService::ServiceBackend *AndroidServicePlugin::createServiceBackend(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("android"))
		return new AndroidServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *AndroidServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	auto parts = detectNamedService(serviceId);
	if(!parts.first.isEmpty())
		serviceId = parts.second + QLatin1Char('.') + parts.first;

	if(backend == QStringLiteral("android"))
		return new AndroidServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
