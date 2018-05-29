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
	bool isAutostartEnabled() const override;

	QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args) override;

public slots:
	bool start() override;
	bool stop() override;
	bool reload() override;
	bool enableAutostart() override;
	bool disableAutostart() override;

protected:
	QString serviceName() const override;

private:
	mutable bool _existsRefBase = false;
	mutable bool *_exists = nullptr;

	int runSystemctl(const QByteArray &command,
					 const QStringList &extraArgs = {},
					 QByteArray *outData = nullptr,
					 bool noPrepare = false) const;
};

#endif // SYSTEMDSERVICECONTROL_H
