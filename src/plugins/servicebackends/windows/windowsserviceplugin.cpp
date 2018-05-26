#include "windowsserviceplugin.h"
#include "windowsservicebackend.h"

WindowsServicePlugin::WindowsServicePlugin(QObject *parent) :
	QObject(parent)
{}

QtService::ServiceBackend *WindowsServicePlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == QStringLiteral("windows"))
		return new WindowsServiceBackend(parent);
	else
		return nullptr;
}
