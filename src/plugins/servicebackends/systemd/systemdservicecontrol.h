#ifndef SYSTEMDSERVICECONTROL_H
#define SYSTEMDSERVICECONTROL_H

#include <QtService/ServiceControl>

class SystemdServiceControl : public QtService::ServiceControl
{
	Q_OBJECT

public:
	explicit SystemdServiceControl(QString &&serviceId, QObject *parent = nullptr);

	QString backend() const override;
	SupportFlags supportFlags() const override;
	bool serviceExists() const override;
	ServiceStatus status() const override;
	bool isEnabled() const override;

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
					 QByteArray *outData = nullptr,
					 bool noPrepare = false) const;
};

#endif // SYSTEMDSERVICECONTROL_H
