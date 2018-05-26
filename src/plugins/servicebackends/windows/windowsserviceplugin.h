#ifndef WINDOWSSERVICEPLUGIN_H
#define WINDOWSSERVICEPLUGIN_H

#include <QtService/ServicePlugin>

class WindowsServicePlugin : public QObject, public QtService::ServicePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtService_ServicePlugin_Iid FILE "windows.json")
	Q_INTERFACES(QtService::ServicePlugin)

public:
	WindowsServicePlugin(QObject *parent = nullptr);

	QtService::ServiceBackend *createInstance(const QString &provider, QObject *parent) override;
};

#endif // WINDOWSSERVICEPLUGIN_H
