#include "standardserviceplugin.h"
#include "standardservicebackend.h"

StandardServicePlugin::StandardServicePlugin(QObject *parent) :
	QObject{parent}
{}

QtService::ServiceBackend *StandardServicePlugin::createInstance(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("standard"))
		return new StandardServiceBackend{service};
	else
		return nullptr;
}
