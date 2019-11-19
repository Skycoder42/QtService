#ifndef LAUNCHDSERVICECONTROL_H
#define LAUNCHDSERVICECONTROL_H

#include <QtCore/QLoggingCategory>

#include <QtService/ServiceControl>

class LaunchdServiceControl : public QtService::ServiceControl
{
	Q_OBJECT

public:
	explicit LaunchdServiceControl(QString &&serviceId, QObject *parent = nullptr);

	QString backend() const override;
	SupportFlags supportFlags() const override;
	bool serviceExists() const override;
	bool isEnabled() const override;
	QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args) override;

public Q_SLOTS:
	bool start() override;
	bool stop() override;
	bool setEnabled(bool enabled) override;

protected:
	QString serviceName() const override;

private:
	int runLaunchctl(const QByteArray &command,
					 const QStringList &extraArgs = {},
					 bool withServiceId = true,
					 QByteArray *outData = nullptr) const;
};

Q_DECLARE_LOGGING_CATEGORY(logControl)

#endif // LAUNCHDSERVICECONTROL_H
