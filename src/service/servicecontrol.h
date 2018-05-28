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

	Q_PROPERTY(QString serviceId READ serviceId CONSTANT)

	Q_PROPERTY(SupportFlags supportFlags READ supportFlags CONSTANT)
	Q_PROPERTY(ServiceStatus status READ status)

	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)

public:
	enum SupportFlag {
		SupportsStart = 0x0001,
		SupportsStop = 0x0002,
		SupportsPause = 0x0004,
		SupportsResume = 0x0008,
		SupportsReload = 0x0010,
		SupportsEnable = 0x0020,
		SupportsDisable = 0x0040,

		SupportsStatus = 0x0080,
		SupportsCustomCommands = 0x0100,

		SupportsStartStop = (SupportsStart | SupportsStop),
		SupportsPauseResume = (SupportsPause | SupportsResume),
		SupportsEnableDisable = (SupportsEnable | SupportsDisable)
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
		ServiceStopping
	};
	Q_ENUM(ServiceStatus)

	static ServiceControl *create(const QString &backend, QString serviceId, QObject *parent = nullptr);

	explicit ServiceControl(QString &&serviceId, QObject *parent = nullptr);
	~ServiceControl() override;

	QString serviceId() const;
	virtual SupportFlags supportFlags() const = 0;
	virtual ServiceStatus status() const;
	virtual bool isEnabled() const;

	virtual QString backend() const = 0;

	virtual QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args);
	template <typename TRet, typename... TArgs>
	TRet callCommand(const QByteArray &kind, TArgs... args);
	template <typename... TArgs>
	void callCommand(const QByteArray &kind, TArgs... args);

public Q_SLOTS:
	virtual void start();
	virtual void stop();

	virtual void pause();
	virtual void resume();

	virtual void reload();

	virtual void enable();
	virtual void disable();

	void setEnabled(bool enabled);

protected:
	virtual QString serviceName() const;
	QDir runtimeDir() const;

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
