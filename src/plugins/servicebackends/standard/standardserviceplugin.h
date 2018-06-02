#ifndef STANDARDSERVICEPLUGIN_H
#define STANDARDSERVICEPLUGIN_H

#include <QtService/ServicePlugin>
#include <QtCore/QLoggingCategory>

class StandardServicePlugin : public QObject, public QtService::ServicePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtService_ServicePlugin_Iid FILE "standard.json")
	Q_INTERFACES(QtService::ServicePlugin)

public:
	StandardServicePlugin(QObject *parent = nullptr);

	QString currentServiceId() const override;
	QtService::ServiceBackend *createServiceBackend(const QString &provider, QtService::Service *service) override;
	QtService::ServiceControl *createServiceControl(const QString &backend, QString &&serviceId, QObject *parent) override;
};

Q_DECLARE_LOGGING_CATEGORY(logQtService)

#endif // STANDARDSERVICEPLUGIN_H
