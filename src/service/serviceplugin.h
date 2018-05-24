#ifndef QTSERVICE_SERVICEPLUGIN_H
#define QTSERVICE_SERVICEPLUGIN_H

#include <QtCore/qobject.h>
#include <QtCore/qbytearraylist.h>

#include "QtService/qtservice_global.h"
#include "QtService/service.h"

namespace QtService {

class Q_SERVICE_EXPORT ServiceBackend : public QObject
{
	Q_OBJECT

public:
	ServiceBackend(QObject *parent = nullptr);

	static QByteArrayList rawArguments(int argc, char **argv);

	virtual int runService(Service *service, int &argc, char **argv, int flags) = 0;

protected Q_SLOTS:
	virtual void signalTriggered(int signal);

protected:
	bool registerForSignal(int signal);
	bool unregisterFromSignal(int signal);
};

class Q_SERVICE_EXPORT ServicePlugin
{
	Q_DISABLE_COPY(ServicePlugin)

public:
	inline ServicePlugin() = default;
	virtual inline ~ServicePlugin() = default;

	virtual ServiceBackend *createInstance(const QString &provider, QObject *parent = nullptr) = 0;
};

}

#define QtService_ServicePlugin_Iid "de.skycoder42.QtService.ServicePlugin"
Q_DECLARE_INTERFACE(QtService::ServicePlugin, QtService_ServicePlugin_Iid)

#endif // QTSERVICE_SERVICEPLUGIN_H
