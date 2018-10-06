#ifndef QTSERVICE_SERVICECONTROL_P_H
#define QTSERVICE_SERVICECONTROL_P_H

#include "qtservice_global.h"
#include "servicecontrol.h"

namespace QtService {

class ServiceControlPrivate
{
	Q_DISABLE_COPY(ServiceControlPrivate)
public:
	ServiceControlPrivate(QString &&serviceId);

	QString serviceId;
	bool blocking = true;
	QString error;
};

}

#endif // QTSERVICE_SERVICECONTROL_P_H
