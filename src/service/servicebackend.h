#ifndef QTSERVICE_SERVICEBACKEND_H
#define QTSERVICE_SERVICEBACKEND_H

#include <QtCore/qobject.h>
#include <QtCore/qbytearraylist.h>
#include <QtCore/qscopedpointer.h>

#include "QtService/qtservice_global.h"
#include "QtService/service.h"

namespace QtService {

class ServiceBackendPrivate;
//! The interface that needs to be implemented to provide the backend for the service engine
class Q_SERVICE_EXPORT ServiceBackend : public QObject
{
	Q_OBJECT

public:
	//! The standard service commands that the library can handle
	enum ServiceCommand {
		StartCommand, //!< Service was started. Will lead to Service::onStart beeing called
		StopCommand, //!< Service should stop. Will lead to Service::onStop beeing called
		ReloadCommand, //!< Service should reload. Will lead to Service::onReload beeing called
		PauseCommand, //!< Service should pause. Will lead to Service::onPause beeing called
		ResumeCommand //!< Service was resumed. Will lead to Service::onResume beeing called
	};
	Q_ENUM(ServiceCommand)

	//! Constructor with the service instance the backend was created for
	ServiceBackend(Service *service);
	~ServiceBackend() override;

	//! Is called as the services main function by the library from Service::exec
	virtual int runService(int &argc, char **argv, int flags) = 0;
	//! Is called by Service::quit to stop the service programatically
	virtual void quitService() = 0;
	//! Is called by Service::reload to reload the service programatically
	virtual void reloadService() = 0;

	//! Is called by Service::getSockets and Service::getSocket to get the activated sockets
	virtual QList<int> getActivatedSockets(const QByteArray &name);

protected Q_SLOTS:
	//! Is called by the library if a unix signal or windows console signal was triggered
	virtual void signalTriggered(int signal);

	//! Calls the Service standard methods for the given command code
	void processServiceCommand(QtService::ServiceBackend::ServiceCommand code);
	//! Calls a special command as service callback synchronously
	QVariant processServiceCallbackImpl(const QByteArray &kind, const QVariantList &args = {});

protected:
	//! The Service instance this backend was created with
	QtService::Service *service() const;

	/*! @copybrief QtService::ServiceBackend::processServiceCallbackImpl
	@tparam TRet The return type
	@tparam TArgs Generic arguments types
	@copydetails QtService::ServiceBackend::processServiceCallbackImpl
	*/
	template <typename TRet, typename... TArgs>
	TRet processServiceCallback(const QByteArray &kind, TArgs... args);
	/*! @copybrief QtService::ServiceBackend::processServiceCallbackImpl
	@tparam TArgs Generic arguments types
	@copydetails QtService::ServiceBackend::processServiceCallbackImpl
	*/
	template <typename... TArgs>
	void processServiceCallback(const QByteArray &kind, TArgs... args);

	//! Register for a unix/windows signal your backend wants to handle
	bool registerForSignal(int signal);
	//! Unregister from a unix/windows signal your backend doesn't want to handle anymore
	bool unregisterFromSignal(int signal);

	//! Calls the Service::preStart method synchronously
	bool preStartService();

private Q_SLOTS:
	void onSvcStarted(bool success);
	void onSvcStopped();
	void onSvcResumed(bool success);
	void onSvcPaused(bool success);

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
