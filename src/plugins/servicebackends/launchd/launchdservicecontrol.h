#ifndef LAUNCHDSERVICECONTROL_H
#define LAUNCHDSERVICECONTROL_H

#include <QtService/ServiceControl>

class LaunchdServiceControl : public QtService::ServiceControl
{
	Q_OBJECT

public:
	explicit LaunchdServiceControl(QString &&serviceId, QObject *parent = nullptr);

	QString backend() const override;
	SupportFlags supportFlags() const override;
	bool serviceExists() const override;
	ServiceStatus status() const override;
	QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args) override;

public Q_SLOTS:
	bool start() override;
	bool stop() override;

protected:
	QString serviceName() const override;

private:
	int runLaunchctl(const QByteArray &command,
					 const QStringList &extraArgs = {},
					 QByteArray *outData = nullptr) const;
};

#endif // LAUNCHDSERVICECONTROL_H
