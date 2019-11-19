#include "androidserviceplugin.h"
#include "androidservicebackend.h"
#include "androidservicecontrol.h"

AndroidServicePlugin::AndroidServicePlugin(QObject *parent) :
	QObject{parent}
{}

QString AndroidServicePlugin::findServiceId(const QString &backend, const QString &serviceName, const QString &domain) const
{
	if (backend == QStringLiteral("android"))
		return domain + QLatin1Char('.') + serviceName;
	else
		return {};
}

QtService::ServiceBackend *AndroidServicePlugin::createServiceBackend(const QString &backend, QtService::Service *service)
{
	if (backend == QStringLiteral("android"))
		return new AndroidServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *AndroidServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	if (backend == QStringLiteral("android"))
		return new AndroidServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
