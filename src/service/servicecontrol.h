#ifndef QTSERVICE_SERVICECONTROL_H
#define QTSERVICE_SERVICECONTROL_H

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qdir.h>
#include <QtCore/qvariant.h>
#include <QtCore/qhash.h>

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
	Q_PROPERTY(BlockMode blocking READ blocking NOTIFY blockingChanged)

	//! A string describing the last error that occured
	Q_PROPERTY(QString error READ error RESET clearError NOTIFY errorChanged)

	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled) // clazy:exclude=qproperty-without-notify

public:
	//! Flags that indicate what kind of queries and commands the specific control implementation provides
	enum class SupportFlag {
		Start = 0x0001, //!< Can start services via ServiceControl::start
		Stop = 0x0002, //!< Can stop services via ServiceControl::stop
		Pause = 0x0004, //!< Can pause services via ServiceControl::pause
		Resume = 0x0008, //!< Can resume services via ServiceControl::resume
		Reload = 0x0010, //!< Can reload services via ServiceControl::reload
		GetAutostart = 0x0020, //!< Can read the autostart state of a service
		SetAutostart = 0x0040, //!< Can write the autostart state of a service
		SetBlocking = 0x0080,
		SetEnabled = 0x0100,

		Status = 0x0200, //!< Can read the current status of a service
		CustomCommands = 0x0400, //!< Supports the execution of specific custom commands

		StartStop = (Start | Stop), //!< SupportFlag::Start | SupportFlag::Stop
		PauseResume = (Pause | Resume), //!< SupportFlag::Pause | SupportFlag::Resume
		Autostart = (GetAutostart | SetAutostart), //!< SupportFlag::GetAutostart | SupportFlag::SetAutostart
	};
	Q_DECLARE_FLAGS(SupportFlags, SupportFlag)
	Q_FLAG(SupportFlags)

	//! The different states a service can be in
	enum class Status {
		Unknown = 0, //!< The control is unable to determine the services state

		Stopped, //!< The service is not running
		Starting, //!< The service is currently trying to start
		Running, //!< The service is running
		Pausing, //!< The service is currently trying to pause
		Paused, //!< The service is paused
		Resuming, //!< The service is currently trying to resume
		Reloading, //!< The service is currently reloading it's configuration
		Stopping, //!< The service is currently trying to stop
		Errored //!< The service is not running and exited in an error state
	};
	Q_ENUM(Status)

	enum class BlockMode {
		Undetermined,
		Blocking,
		NonBlocking
	};
	Q_ENUM(BlockMode)

	//! Returns a list of all available backends
	static QStringList listBackends();
	//! Returns the backend that is most likely to be used on the current platform
	static QString likelyBackend();
	static QString serviceIdFromName(const QString &backend, const QString &name);
	static QString serviceIdFromName(const QString &backend, const QString &name, const QString &domain);
	//! Creates a new ServiceControl for the given service on the service manager defined by backend
	static ServiceControl *create(const QString &backend, QString serviceId, QObject *parent = nullptr);
	//! @copybrief ServiceControl::create(const QString &, QString, QObject *)
	static ServiceControl *create(const QString &backend, QString serviceId, QString serviceNameOverride, QObject *parent = nullptr);
	//! Creates a new ServiceControl by guessing the service id from the given name and this applications domain
	static ServiceControl *createFromName(const QString &backend, const QString &serviceName, QObject *parent = nullptr);
	//! Creates a new ServiceControl by guessing the service id from the given name and domain
	static ServiceControl *createFromName(const QString &backend, const QString &serviceName, const QString &domain, QObject *parent = nullptr);

	~ServiceControl() override;

	//! @readAcFn{ServiceControl::backend}
	virtual QString backend() const = 0;
	//! @readAcFn{ServiceControl::serviceId}
	QString serviceId() const;
	//! @readAcFn{ServiceControl::supportFlags}
	virtual SupportFlags supportFlags() const = 0;
	//! @readAcFn{ServiceControl::blocking}
	virtual BlockMode blocking() const;
	//! @readAcFn{ServiceControl::error}
	QString error() const;
	//! @readAcFn{ServiceControl::enabled}
	virtual bool isEnabled() const;

	//! Calls the command of kind with the given arguments and returns it's result
	Q_INVOKABLE virtual QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args = {});

	/*! @copybrief ServiceControl::callGenericCommand
	 * @tparam TRet The return value of the command
	 * @tparam TArgs The arguments passed to the command
	 * @copydetails ServiceControl::callGenericCommand
	 *
	 * This generic variant will automatically convert the arguments to variant types and
	 * pass the to the callGenericCommand() method.
	 *
	 * @sa @ref qtservice_backends, ServiceControl::callGenericCommand, ServiceControl::blocking, Service::onCallback, Service::addCallback
	 */
	template <typename TRet, typename... TArgs>
	TRet callCommand(const QByteArray &kind, TArgs... args);
	/*! @copybrief ServiceControl::callGenericCommand
	 * @tparam TArgs The arguments passed to the command
	 * @copydetails ServiceControl::callGenericCommand
	 *
	 * This generic variant will automatically convert the arguments to variant types and
	 * pass the to the callGenericCommand() method.
	 *
	 * @sa @ref qtservice_backends, ServiceControl::callGenericCommand, ServiceControl::blocking, Service::onCallback, Service::addCallback
	 */
	template <typename... TArgs>
	void callCommand(const QByteArray &kind, TArgs... args);

	//! Checks if the service this control was created for actually exists
	Q_INVOKABLE virtual bool serviceExists() const = 0;
	//! Returns the current status of the service
	Q_INVOKABLE virtual QtService::ServiceControl::Status status() const;
	//! Returns the current autostart state of the service
	Q_INVOKABLE virtual bool isAutostartEnabled() const;
	//! Returns the runtime directory of this controls service
	Q_INVOKABLE QDir runtimeDir() const;

public Q_SLOTS:
	//! Send a start command for the controls service to the service manager
	virtual bool start();
	//! Send a stop command for the controls service to the service manager
	virtual bool stop();
	virtual bool restart();

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
	virtual bool setBlocking(bool blocking);
	//! @resetAcFn{ServiceControl::error}
	void clearError();
	//! @writeAcFn{ServiceControl::enabled}
	virtual bool setEnabled(bool enabled);

Q_SIGNALS:
	//! @notifyAcFn{ServiceControl::blocking}
	void blockingChanged(BlockMode blocking);
	//! @notifyAcFn{ServiceControl::error}
	void errorChanged(QString error, QPrivateSignal);

protected:
	//! @private
	explicit ServiceControl(QString &&serviceId, QObject *parent = nullptr);

	//! Returns the common name of the controls service
	virtual QString serviceName() const;
	//! Returns the common name of the controls service, with a possible override on creation
	QString realServiceName() const;

	//! @writeAcFn{ServiceControl::error}
	void setError(QString error) const;

private:
	friend class QtService::ServiceControlPrivate;
	QScopedPointer<ServiceControlPrivate> d;
};

Q_DECL_CONST_FUNCTION Q_DECL_CONSTEXPR inline uint qHash(QtService::ServiceControl::SupportFlags key, uint seed = 0) Q_DECL_NOTHROW {
	return ::qHash(static_cast<int>(key), seed);
}
Q_DECL_CONST_FUNCTION Q_DECL_CONSTEXPR inline uint qHash(QtService::ServiceControl::Status key, uint seed = 0) Q_DECL_NOTHROW {
	return ::qHash(static_cast<int>(key), seed);
}
Q_DECL_CONST_FUNCTION Q_DECL_CONSTEXPR inline uint qHash(QtService::ServiceControl::BlockMode key, uint seed = 0) Q_DECL_NOTHROW {
	return ::qHash(static_cast<int>(key), seed);
}

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
