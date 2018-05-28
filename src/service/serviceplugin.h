#ifndef QTSERVICE_SERVICEPLUGIN_H
#define QTSERVICE_SERVICEPLUGIN_H

#include <QtCore/qobject.h>

#include "QtService/qtservice_global.h"

namespace QtService {

class Service;
class ServiceBackend;
class ServiceControl;
class Q_SERVICE_EXPORT ServicePlugin
{
	Q_DISABLE_COPY(ServicePlugin)

public:
	inline ServicePlugin() = default;
	virtual inline ~ServicePlugin() = default;

	virtual ServiceBackend *createServiceBackend(const QString &provider, Service *service) = 0;
	virtual ServiceControl *createServiceControl(const QString &backend, QString &&serviceId, QObject *parent) = 0;
};

}

#define QtService_ServicePlugin_Iid "de.skycoder42.QtService.ServicePlugin"
Q_DECLARE_INTERFACE(QtService::ServicePlugin, QtService_ServicePlugin_Iid)

#endif // QTSERVICE_SERVICEPLUGIN_H
