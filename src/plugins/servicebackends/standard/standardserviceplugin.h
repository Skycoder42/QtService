#ifndef STANDARDSERVICEPLUGIN_H
#define STANDARDSERVICEPLUGIN_H

#include <QtService/ServicePlugin>

class StandardServicePlugin : public QObject, public QtService::ServicePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtService_ServicePlugin_Iid FILE "standard.json")
	Q_INTERFACES(QtService::ServicePlugin)

public:
	StandardServicePlugin(QObject *parent = nullptr);

	QtService::ServiceBackend *createInstance(const QString &provider, QtService::Service *service) override;
};

#endif // STANDARDSERVICEPLUGIN_H
