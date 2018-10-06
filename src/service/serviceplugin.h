#ifndef QTSERVICE_SERVICEPLUGIN_H
#define QTSERVICE_SERVICEPLUGIN_H

#include <QtCore/qobject.h>

#include "QtService/qtservice_global.h"

namespace QtService {

class Service;
class ServiceBackend;
class ServiceControl;
//! The plugin interface to implement as primary interface of a servicebackend plugin
class Q_SERVICE_EXPORT ServicePlugin
{
	Q_DISABLE_COPY(ServicePlugin)

public:
	ServicePlugin();
	virtual ~ServicePlugin();

	//! Return the ID of the currently setup service
	virtual QString currentServiceId() const = 0;

	//! Create a new service backend for the given backend and service
	virtual ServiceBackend *createServiceBackend(const QString &backend, Service *service) = 0;
	//! Create a new service backend for the given backend, name and parent
	virtual ServiceControl *createServiceControl(const QString &backend, QString &&serviceId, QObject *parent) = 0;

protected:
	QPair<QString, QString> detectNamedService(const QString &serviceId) const;
};

}

//! The IID to be used to create a service plugin
#define QtService_ServicePlugin_Iid "de.skycoder42.QtService.ServicePlugin"
Q_DECLARE_INTERFACE(QtService::ServicePlugin, QtService_ServicePlugin_Iid)

//! @file serviceplugin.h The ServicePlugin header
#endif // QTSERVICE_SERVICEPLUGIN_H
