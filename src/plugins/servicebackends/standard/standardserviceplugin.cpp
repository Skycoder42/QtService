#include "standardserviceplugin.h"
#include "standardservicebackend.h"
#include "standardservicecontrol.h"

Q_LOGGING_CATEGORY(logQtService, "qtservice.servicebackends.standard"); //TODO, QtInfoMsg);

StandardServicePlugin::StandardServicePlugin(QObject *parent) :
	QObject{parent}
{}

QString StandardServicePlugin::currentServiceId() const
{
	return QCoreApplication::applicationFilePath();
}

QtService::ServiceBackend *StandardServicePlugin::createServiceBackend(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("standard"))
		return new StandardServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *StandardServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	if(backend == QStringLiteral("standard"))
		return new StandardServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
