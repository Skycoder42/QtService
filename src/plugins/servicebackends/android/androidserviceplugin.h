#ifndef ANDROIDSERVICEPLUGIN_H
#define ANDROIDSERVICEPLUGIN_H

#include <QtService/ServicePlugin>
#include <QtCore/QLoggingCategory>

#include <QtAndroidExtras/QAndroidIntent>
#include <QtAndroidExtras/QAndroidBinder>
#include <QtAndroidExtras/QAndroidServiceConnection>
#include <QtAndroidExtras/QtAndroid>

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

Q_DECLARE_LOGGING_CATEGORY(logQtService)

Q_DECLARE_METATYPE(QAndroidBinder*)
Q_DECLARE_METATYPE(QAndroidIntent)
Q_DECLARE_METATYPE(QAndroidServiceConnection*)
Q_DECLARE_METATYPE(QtAndroid::BindFlags)

#endif // ANDROIDSERVICEPLUGIN_H
