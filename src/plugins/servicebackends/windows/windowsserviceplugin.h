#ifndef WINDOWSSERVICEPLUGIN_H
#define WINDOWSSERVICEPLUGIN_H

#include <QtService/ServicePlugin>
#include <QtCore/QLoggingCategory>

class WindowsServicePlugin : public QObject, public QtService::ServicePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtService_ServicePlugin_Iid FILE "windows.json")
	Q_INTERFACES(QtService::ServicePlugin)

public:
	WindowsServicePlugin(QObject *parent = nullptr);

	QString findServiceId(const QString &backend, const QString &serviceName, const QString &domain) const override;
	QtService::ServiceBackend *createServiceBackend(const QString &backend, QtService::Service *service) override;
	QtService::ServiceControl *createServiceControl(const QString &backend, QString &&serviceId, QObject *parent) override;
};

Q_DECLARE_LOGGING_CATEGORY(logQtService)

#endif // WINDOWSSERVICEPLUGIN_H
