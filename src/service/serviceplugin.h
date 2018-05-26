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
	enum ServiceCommand {
		StartCommand,
		StopCommand,
		ReloadCommand,
		PauseCommand,
		ResumeCommand
	};
	Q_ENUM(ServiceCommand)

	ServiceBackend(QObject *parent = nullptr);

	virtual int runService(Service *service, int &argc, char **argv, int flags) = 0;
	virtual void quitService() = 0;
	virtual void reloadService() = 0;

	virtual QHash<int, QByteArray> getActivatedSockets();

protected Q_SLOTS:
	virtual void signalTriggered(int signal);

	void processServiceCommand(QtService::Service *service, QtService::ServiceBackend::ServiceCommand code);
	QVariant processServiceCallbackImpl(QtService::Service *service, const QByteArray &kind, const QVariantList &args = {});

protected:
	template <typename TRet, typename... TArgs>
	TRet processServiceCallback(QtService::Service *service, const QByteArray &kind, TArgs... args);
	template <typename... TArgs>
	void processServiceCallback(QtService::Service *service, const QByteArray &kind, TArgs... args);

	bool registerForSignal(int signal);
	bool unregisterFromSignal(int signal);

	bool preStartService(Service *service);
private:
};

class Q_SERVICE_EXPORT ServicePlugin
{
	Q_DISABLE_COPY(ServicePlugin)

public:
	inline ServicePlugin() = default;
	virtual inline ~ServicePlugin() = default;

	virtual ServiceBackend *createInstance(const QString &provider, QObject *parent = nullptr) = 0;
};

template<typename TRet, typename... TArgs>
TRet ServiceBackend::processServiceCallback(Service *service, const QByteArray &kind, TArgs... args)
{
	return processServiceCallbackImpl(service, kind, {QVariant::fromValue(args)...}).template value<TRet>();
}

template<typename... TArgs>
void ServiceBackend::processServiceCallback(Service *service, const QByteArray &kind, TArgs... args)
{
	processServiceCallbackImpl(service, kind, {QVariant::fromValue(args)...});
}


}

Q_DECLARE_METATYPE(QtService::ServiceBackend::ServiceCommand)
#define QtService_ServicePlugin_Iid "de.skycoder42.QtService.ServicePlugin"
Q_DECLARE_INTERFACE(QtService::ServicePlugin, QtService_ServicePlugin_Iid)

#endif // QTSERVICE_SERVICEPLUGIN_H
