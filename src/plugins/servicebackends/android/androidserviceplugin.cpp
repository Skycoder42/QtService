#include "androidserviceplugin.h"
#include "androidservicebackend.h"

AndroidServicePlugin::AndroidServicePlugin(QObject *parent) :
	QObject{parent}
{}

QtService::ServiceBackend *AndroidServicePlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == QStringLiteral("android"))
		return new AndroidServiceBackend(parent);
	else
		return nullptr;
}
