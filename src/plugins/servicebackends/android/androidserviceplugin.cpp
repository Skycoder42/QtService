#include "androidserviceplugin.h"
#include "androidservicebackend.h"
#include "androidservicecontrol.h"

Q_LOGGING_CATEGORY(logQtService, "qtservice.servicebackends.android"); //TODO, QtInfoMsg);

AndroidServicePlugin::AndroidServicePlugin(QObject *parent) :
	QObject{parent}
{}

QString AndroidServicePlugin::currentServiceId() const
{
	auto service = QtAndroid::androidService();
	if(service.isValid()) {
		auto clazz = service.callObjectMethod("getClass", "()Ljava/lang/Class;");
		return clazz.callObjectMethod("getName", "()Ljava/lang/String;").toString();
	} else
		return {};
}

QtService::ServiceBackend *AndroidServicePlugin::createServiceBackend(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("android"))
		return new AndroidServiceBackend{service};
	else
		return nullptr;
}

QtService::ServiceControl *AndroidServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	if(backend == QStringLiteral("android"))
		return new AndroidServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
