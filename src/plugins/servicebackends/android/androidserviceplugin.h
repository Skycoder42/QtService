#ifndef ANDROIDSERVICEPLUGIN_H
#define ANDROIDSERVICEPLUGIN_H

#include <QtService/ServicePlugin>

class AndroidServicePlugin : public QObject, public QtService::ServicePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtService_ServicePlugin_Iid FILE "android.json")
	Q_INTERFACES(QtService::ServicePlugin)

public:
	AndroidServicePlugin(QObject *parent = nullptr);

	QString currentServiceId() const override;
	QtService::ServiceBackend *createServiceBackend(const QString &provider, QtService::Service *service) override;
	QtService::ServiceControl *createServiceControl(const QString &backend, QString &&serviceId, QObject *parent) override;
};

#endif // ANDROIDSERVICEPLUGIN_H
