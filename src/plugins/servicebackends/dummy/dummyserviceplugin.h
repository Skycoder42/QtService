#ifndef DUMMYSERVICEPLUGIN_H
#define DUMMYSERVICEPLUGIN_H

#include <QtService/ServicePlugin>

class DummyServicePlugin : public QObject, public QtService::ServicePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtService_ServicePlugin_Iid FILE "dummy.json")
	Q_INTERFACES(QtService::ServicePlugin)

public:
	DummyServicePlugin(QObject *parent = nullptr);

	QtService::ServiceBackend *createInstance(const QString &provider, QObject *parent) override;
};

#endif // DUMMYSERVICEPLUGIN_H
