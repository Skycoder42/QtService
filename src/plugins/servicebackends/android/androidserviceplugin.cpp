#include "androidserviceplugin.h"
#include "androidservicebackend.h"

AndroidServicePlugin::AndroidServicePlugin(QObject *parent) :
	QObject{parent}
{}

QtService::ServiceBackend *AndroidServicePlugin::createServiceBackend(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("android"))
		return new AndroidServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *AndroidServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	return nullptr;
}
