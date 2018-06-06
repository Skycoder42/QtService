#include "qmlservicesingleton.h"
using namespace QtService;

QmlServiceSingleton::QmlServiceSingleton(QObject *parent) :
	QObject(parent)
{}

ServiceControl *QmlServiceSingleton::createControl(const QString &backend, QString serviceId, QObject *parent) const
{
	return ServiceControl::create(backend, std::move(serviceId), parent);
}

Service *QmlServiceSingleton::service() const
{
	return Service::instance();
}
