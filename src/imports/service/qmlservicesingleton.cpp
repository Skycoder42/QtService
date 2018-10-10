#include "qmlservicesingleton.h"
using namespace QtService;

QmlServiceSingleton::QmlServiceSingleton(QObject *parent) :
	QObject(parent)
{}

ServiceControl *QmlServiceSingleton::createControl(const QString &backend, QString serviceId, QObject *parent) const
{
	return ServiceControl::create(backend, std::move(serviceId), parent);
}

ServiceControl *QmlServiceSingleton::createControlFromName(const QString &backend, const QString &serviceName, QObject *parent) const
{
	return ServiceControl::createFromName(backend, serviceName, parent);
}

ServiceControl *QmlServiceSingleton::createControlFromName(const QString &backend, const QString &serviceName, const QString &domain, QObject *parent) const
{
	return ServiceControl::createFromName(backend, serviceName, domain, parent);
}

Service *QmlServiceSingleton::service() const
{
	return Service::instance();
}
