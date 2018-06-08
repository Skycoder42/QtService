#ifndef QTSERVICE_SERVICECONTROL_H
#define QTSERVICE_SERVICECONTROL_H

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qdir.h>
#include <QtCore/qvariant.h>
#include <QtCore/qexception.h>

#include "QtService/qtservice_global.h"

namespace QtService {

class ServiceControlPrivate;
//! A class to interact with the systems service manager
class Q_SERVICE_EXPORT ServiceControl : public QObject
{
	Q_OBJECT

	//! The service backend this control interacts with
	Q_PROPERTY(QString backend READ backend CONSTANT)
	//! The ID of the service this control manages
	Q_PROPERTY(QString serviceId READ serviceId CONSTANT)

	//! The features that this implementation supports
	Q_PROPERTY(QtService::ServiceControl::SupportFlags supportFlags READ supportFlags CONSTANT)
	//! Specifies if service control commands are blocking
	Q_PROPERTY(bool blocking READ isBlocking WRITE setBlocking NOTIFY blockingChanged)

	//! A string describing the last error that occured
	Q_PROPERTY(QString error READ error RESET clearError NOTIFY errorChanged)

public:
	//! Flags that indicate what kind of queries and commands the specific control implementation provides
	enum SupportFlag {
		SupportsStart = 0x0001, //!< Can start services via ServiceControl::start
		SupportsStop = 0x0002, //!< Can stop services via ServiceControl::stop
		SupportsPause = 0x0004, //!< Can pause services via ServiceControl::pause
		SupportsResume = 0x0008, //!< Can resume services via ServiceControl::resume
		SupportsReload = 0x0010, //!< Can reload services via ServiceControl::reload
		SupportsGetAutostart = 0x0020, //!< Can read the autostart state of a service
		SupportsSetAutostart = 0x0040, //!< Can write the autostart state of a service
		SupportsBlocking = 0x0080, //!< Supports service commands that block for the execution
		SupportsNonBlocking = 0x0100, //!< Supports service commands the only trigger execution without blocking

		SupportsStatus = 0x0200, //!< Can read the current status of a service
		SupportsCustomCommands = 0x0400, //!< Supports the execution of specific custom commands

		SupportsStartStop = (SupportsStart | SupportsStop), //!< SupportsStart | SupportsStop
		SupportsPauseResume = (SupportsPause | SupportsResume), //!< SupportsPause | SupportsResume
		SupportsAutostart = (SupportsGetAutostart | SupportsSetAutostart), //!< SupportsGetAutostart | SupportsSetAutostart
		SupportsBlockingNonBlocking = (SupportsBlocking | SupportsNonBlocking) //!< SupportsBlocking | SupportsNonBlocking
	};
	Q_DECLARE_FLAGS(SupportFlags, SupportFlag)
	Q_FLAG(SupportFlags)

	//! The different states a service can be in
	enum ServiceStatus {
		ServiceStatusUnknown = 0, //!< The control is unable to determine the services state

		ServiceStopped, //!< The service is not running
		ServiceStarting, //!< The service is currently trying to start
		ServiceRunning, //!< The service is running
		ServicePausing, //!< The service is currently trying to pause
		ServicePaused, //!< The service is paused
		ServiceResuming, //!< The service is currently trying to resume
		ServiceReloading, //!< The service is currently reloading it's configuration
		ServiceStopping, //!< The service is currently trying to stop
		ServiceErrored //!< The service is not running and exited in an error state
	};
	Q_ENUM(ServiceStatus)

	//! Returns a list of all available backends
	static QStringList listBackends();
	//! Returns the backend that is most likely to be used on the current platform
	static QString likelyBackend();
	//! Creates a new ServiceControl for the given service on the service manager defined by backend
	static ServiceControl *create(const QString &backend, QString serviceId, QObject *parent = nullptr);

	//! @private
	explicit ServiceControl(QString &&serviceId, QObject *parent = nullptr);
	~ServiceControl() override;

	//! @readAcFn{ServiceControl::backend}
	virtual QString backend() const = 0;
	//! @readAcFn{ServiceControl::serviceId}
	QString serviceId() const;
	//! @readAcFn{ServiceControl::supportFlags}
	virtual SupportFlags supportFlags() const = 0;
	//! @readAcFn{ServiceControl::blocking}
	bool isBlocking() const;
	//! @readAcFn{ServiceControl::error}
	QString error() const;

	//! Calls the command of kind with the given arguments and returns it's result
	Q_INVOKABLE virtual QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args);
	//! @copybrief ServiceControl::callGenericCommand
	template <typename TRet, typename... TArgs>
	TRet callCommand(const QByteArray &kind, TArgs... args);
	//! @copybrief ServiceControl::callGenericCommand
	template <typename... TArgs>
	void callCommand(const QByteArray &kind, TArgs... args);

	//! Checks if the service this control was created for actually exists
	Q_INVOKABLE virtual bool serviceExists() const = 0;
	//! Returns the current status of the service
	Q_INVOKABLE virtual QtService::ServiceControl::ServiceStatus status() const;
	//! Returns the current autostart state of the service
	Q_INVOKABLE virtual bool isAutostartEnabled() const;
	//! Returns the runtime directory of this controls service
	Q_INVOKABLE QDir runtimeDir() const;

public Q_SLOTS:
	//! Send a start command for the controls service to the service manager
	virtual bool start();
	//! Send a stop command for the controls service to the service manager
	virtual bool stop();

	//! Send a pause command for the controls service to the service manager
	virtual bool pause();
	//! Send a resume command for the controls service to the service manager
	virtual bool resume();

	//! Send a reload command for the controls service to the service manager
	virtual bool reload();

	//! Enables autostart for the controls service
	virtual bool enableAutostart();
	//! Disables autostart for the controls service
	virtual bool disableAutostart();

	//! @writeAcFn{ServiceControl::blocking}
	void setBlocking(bool blocking);
	//! @resetAcFn{ServiceControl::error}
	void clearError();

Q_SIGNALS:
	//! @notifyAcFn{ServiceControl::blocking}
	void blockingChanged(bool blocking, QPrivateSignal);
	//! @notifyAcFn{ServiceControl::error}
	void errorChanged(QString error, QPrivateSignal);

protected:
	//! Returns the common name of the controls service
	virtual QString serviceName() const;

	//! @writeAcFn{ServiceControl::error}
	void setError(QString error) const;

private:
	QScopedPointer<ServiceControlPrivate> d;
};

// ------------- Generic Implementations -------------

template<typename TRet, typename... TArgs>
TRet ServiceControl::callCommand(const QByteArray &kind, TArgs... args)
{
	return callGenericCommand(kind, {QVariant::fromValue(args)...}).template value<TRet>();
}

template<typename... TArgs>
void ServiceControl::callCommand(const QByteArray &kind, TArgs... args)
{
	callGenericCommand(kind, {QVariant::fromValue(args)...});
}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(QtService::ServiceControl::SupportFlags)

#endif // QTSERVICE_SERVICECONTROL_H
