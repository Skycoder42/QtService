#ifndef QTSERVICE_SERVICEBACKEND_H
#define QTSERVICE_SERVICEBACKEND_H

#include <QtCore/qobject.h>
#include <QtCore/qbytearraylist.h>
#include <QtCore/qscopedpointer.h>

#include "QtService/qtservice_global.h"
#include "QtService/service.h"

namespace QtService {

class ServiceBackendPrivate;
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

	ServiceBackend(Service *service);
	~ServiceBackend() override;

	virtual int runService(int &argc, char **argv, int flags) = 0;
	virtual void quitService() = 0;
	virtual void reloadService() = 0;

	virtual QList<int> getActivatedSockets(const QByteArray &name);

protected Q_SLOTS:
	virtual void signalTriggered(int signal);

	void processServiceCommand(QtService::ServiceBackend::ServiceCommand code);
	QVariant processServiceCallbackImpl(const QByteArray &kind, const QVariantList &args = {});

protected:
	QtService::Service *service() const;

	template <typename TRet, typename... TArgs>
	TRet processServiceCallback(const QByteArray &kind, TArgs... args);
	template <typename... TArgs>
	void processServiceCallback(const QByteArray &kind, TArgs... args);

	bool registerForSignal(int signal);
	bool unregisterFromSignal(int signal);

	bool preStartService();

private Q_SLOTS:
	void onSvcStarted();
	void onSvcStopped();
	void onSvcResumed();
	void onSvcPaused();

private:
	QScopedPointer<ServiceBackendPrivate> d;
};

template<typename TRet, typename... TArgs>
TRet ServiceBackend::processServiceCallback(const QByteArray &kind, TArgs... args)
{
	return processServiceCallbackImpl(kind, {QVariant::fromValue(args)...}).template value<TRet>();
}

template<typename... TArgs>
void ServiceBackend::processServiceCallback(const QByteArray &kind, TArgs... args)
{
	processServiceCallbackImpl(kind, {QVariant::fromValue(args)...});
}

}

Q_DECLARE_METATYPE(QtService::ServiceBackend::ServiceCommand)

#endif // QTSERVICE_SERVICEBACKEND_H
