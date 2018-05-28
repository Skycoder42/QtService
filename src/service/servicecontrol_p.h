#ifndef QTSERVICE_SERVICECONTROL_P_H
#define QTSERVICE_SERVICECONTROL_P_H

#include "qtservice_global.h"
#include "servicecontrol.h"

namespace QtService {

class ServiceControlPrivate
{
public:
	ServiceControlPrivate(QString &&serviceId);

	QString serviceId;
};

}

#endif // QTSERVICE_SERVICECONTROL_P_H
