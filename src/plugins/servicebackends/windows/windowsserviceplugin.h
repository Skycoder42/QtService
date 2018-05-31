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

	QString currentServiceId() const override;
	QtService::ServiceBackend *createServiceBackend(const QString &provider, QtService::Service *service) override;
	QtService::ServiceControl *createServiceControl(const QString &backend, QString &&serviceId, QObject *parent) override;
};

#endif // WINDOWSSERVICEPLUGIN_H
