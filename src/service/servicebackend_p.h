#ifndef QTSERVICE_SERVICEBACKEND_P_H
#define QTSERVICE_SERVICEBACKEND_P_H

#include "servicebackend.h"

#include <QtCore/QLoggingCategory>

namespace QtService {

class ServiceBackendPrivate
{
	Q_DISABLE_COPY(ServiceBackendPrivate)
public:
	ServiceBackendPrivate(Service *service);

	Service *service;
	bool operating = false;
};

Q_DECLARE_LOGGING_CATEGORY(logBackend)  // MAJOR make virtual in public part

}

#endif // QTSERVICE_SERVICEBACKEND_P_H
