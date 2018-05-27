#ifndef QTSERVICE_SERVICE_P_H
#define QTSERVICE_SERVICE_P_H

#include <QtCore/QPointer>

#include "service.h"
#include "servicebackend.h"

namespace QtService {

class ServicePrivate
{
public:
	ServicePrivate(int &argc, char **argv, int flags);

	static QPointer<Service> instance;

	int &argc;
	char **argv;
	int flags;

	QString backendProvider;
	ServiceBackend *backend = nullptr;
	QHash<QByteArray, std::function<QVariant(QVariantList)>> callbacks;

	bool wasPaused = false;
};

}

#endif // QTSERVICE_SERVICE_P_H
