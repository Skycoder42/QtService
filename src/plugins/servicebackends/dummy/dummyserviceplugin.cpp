#include "dummyserviceplugin.h"
#include "dummyservicebackend.h"

DummyServicePlugin::DummyServicePlugin(QObject *parent) :
	QObject{parent}
{}

QtService::ServiceBackend *DummyServicePlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == QStringLiteral("dummy"))
		return new DummyServiceBackend(parent);
	else
		return nullptr;
}
