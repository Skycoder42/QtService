#ifndef QTSERVICE_SERVICE_P_H
#define QTSERVICE_SERVICE_P_H

#include <QtCore/QPointer>

#include "service.h"

namespace QtService {

class ServicePrivate
{
public:
	ServicePrivate(int &argc, char **argv, int flags);

	static QPointer<Service> instance;

	int &argc;
	char **argv;
	int flags;
};

}

#endif // QTSERVICE_SERVICE_P_H
