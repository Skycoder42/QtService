#include "androidserviceplugin.h"
#include "androidservicebackend.h"

AndroidServicePlugin::AndroidServicePlugin(QObject *parent) :
	QObject{parent}
{}

QtService::ServiceBackend *AndroidServicePlugin::createInstance(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("android"))
		return new AndroidServiceBackend{service};
	else
		return nullptr;
}
