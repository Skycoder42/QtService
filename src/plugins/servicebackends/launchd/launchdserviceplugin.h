#ifndef LAUNCHDSERVICEPLUGIN_H
#define LAUNCHDSERVICEPLUGIN_H

#include <QtService/ServicePlugin>

class LaunchdServicePlugin : public QObject, public QtService::ServicePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtService_ServicePlugin_Iid FILE "launchd.json")
	Q_INTERFACES(QtService::ServicePlugin)

public:
	LaunchdServicePlugin(QObject *parent = nullptr);

	QtService::ServiceBackend *createInstance(const QString &provider, QtService::Service *service) override;
};

#endif // LAUNCHDSERVICEPLUGIN_H
