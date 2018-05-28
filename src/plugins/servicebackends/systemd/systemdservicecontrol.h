#ifndef SYSTEMDSERVICECONTROL_H
#define SYSTEMDSERVICECONTROL_H

#include <QtService/ServiceControl>

class SystemdServiceControl : public QtService::ServiceControl
{
	Q_OBJECT

public:
	explicit SystemdServiceControl(QString &&serviceId, QObject *parent = nullptr);

	SupportFlags supportFlags() const override;
	ServiceStatus status() const override;
	bool isEnabled() const override;
	QString backend() const override;

	QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args) override;

public slots:
	bool start() override;
	bool stop() override;
	bool reload() override;
	bool enable() override;
	bool disable() override;

protected:
	QString serviceName() const override;

private:
	int runSystemctl(const QByteArray &command,
					 const QStringList &extraArgs = {},
					 QByteArray *outData = nullptr) const;
};

#endif // SYSTEMDSERVICECONTROL_H
