#ifndef QTSERVICE_SERVICECONTROL_H
#define QTSERVICE_SERVICECONTROL_H

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qdir.h>
#include <QtCore/qvariant.h>

#include "QtService/qtservice_global.h"

namespace QtService {

class ServiceControlPrivate;
class Q_SERVICE_EXPORT ServiceControl : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString backend READ backend CONSTANT)
	Q_PROPERTY(QString serviceId READ serviceId CONSTANT)

	Q_PROPERTY(SupportFlags supportFlags READ supportFlags CONSTANT)
	Q_PROPERTY(bool blocking READ isBlocking WRITE setBlocking NOTIFY blockingChanged)

	Q_PROPERTY(bool serviceExists READ serviceExists)
	Q_PROPERTY(ServiceStatus status READ status)
	Q_PROPERTY(bool autostartEnabled READ isAutostartEnabled WRITE setAutostartEnabled)

public:
	enum SupportFlag {
		SupportsStart = 0x0001,
		SupportsStop = 0x0002,
		SupportsPause = 0x0004,
		SupportsResume = 0x0008,
		SupportsReload = 0x0010,
		SupportsGetAutostart = 0x0020,
		SupportsSetAutostart = 0x0040,
		SupportsBlocking = 0x0080,
		SupportsNonBlocking = 0x0100,

		SupportsStatus = 0x0200,
		SupportsCustomCommands = 0x0400,

		SupportsStartStop = (SupportsStart | SupportsStop),
		SupportsPauseResume = (SupportsPause | SupportsResume),
		SupportsAutostart = (SupportsGetAutostart | SupportsSetAutostart),
		SupportsBlockingNonBlocking = (SupportsBlocking | SupportsNonBlocking)
	};
	Q_DECLARE_FLAGS(SupportFlags, SupportFlag)
	Q_FLAG(SupportFlags)

	enum ServiceStatus {
		ServiceStatusUnknown = 0,

		ServiceStopped,
		ServiceStarting,
		ServiceRunning,
		ServicePausing,
		ServicePaused,
		ServiceResuming,
		ServiceReloading,
		ServiceStopping,
		ServiceErrored
	};
	Q_ENUM(ServiceStatus)

	static QStringList listBackends();
	static ServiceControl *create(const QString &backend, QString serviceId, QObject *parent = nullptr);

	explicit ServiceControl(QString &&serviceId, QObject *parent = nullptr);
	~ServiceControl() override;

	virtual QString backend() const = 0;
	QString serviceId() const;
	virtual SupportFlags supportFlags() const = 0;
	bool isBlocking() const;
	virtual bool serviceExists() const = 0;
	virtual ServiceStatus status() const;
	virtual bool isAutostartEnabled() const;

	virtual QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args);
	template <typename TRet, typename... TArgs>
	TRet callCommand(const QByteArray &kind, TArgs... args);
	template <typename... TArgs>
	void callCommand(const QByteArray &kind, TArgs... args);

	QDir runtimeDir() const;

public Q_SLOTS:
	virtual bool start();
	virtual bool stop();

	virtual bool pause();
	virtual bool resume();

	virtual bool reload();

	virtual bool enableAutostart();
	virtual bool disableAutostart();

	void setBlocking(bool blocking);
	bool setAutostartEnabled(bool enabled);

Q_SIGNALS:
	void blockingChanged(bool blocking, QPrivateSignal);

protected:
	virtual QString serviceName() const;

private:
	QScopedPointer<ServiceControlPrivate> d;
};

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
