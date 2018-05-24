#include "standardserviceplugin.h"
#include "standardservicebackend.h"

StandardServicePlugin::StandardServicePlugin(QObject *parent) :
	QObject{parent}
{}

QtService::ServiceBackend *StandardServicePlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == QStringLiteral("standard"))
		return new StandardServiceBackend(parent);
	else
		return nullptr;
}
