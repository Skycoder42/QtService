#ifndef QTSERVICE_SERVICECONTROL_P_H
#define QTSERVICE_SERVICECONTROL_P_H

#include "qtservice_global.h"
#include "servicecontrol.h"

#include <QtCore/QLoggingCategory>

namespace QtService {

class ServiceControlPrivate
{
	Q_DISABLE_COPY(ServiceControlPrivate)

public:
	ServiceControlPrivate(QString &&serviceId);

	QString serviceId;
	QString serviceName;
	bool blocking = true;
	QString error;
};

Q_DECLARE_LOGGING_CATEGORY(logSvcCtrl)

}

#endif // QTSERVICE_SERVICECONTROL_P_H
