#ifndef STANDARDSERVICECONTROL_H
#define STANDARDSERVICECONTROL_H

#include <QtCore/QLockFile>
#include <QtService/ServiceControl>

class StandardServiceControl : public QtService::ServiceControl
{
	Q_OBJECT

public:
	explicit StandardServiceControl(bool debugMode, QString &&serviceId, QObject *parent = nullptr);

	QString backend() const override;
	SupportFlags supportFlags() const override;
	bool serviceExists() const override;
	ServiceStatus status() const override;

public Q_SLOTS:
	bool start() override;
	bool stop() override;

protected:
	QString serviceName() const override;

private:
	const bool _debugMode;
	mutable QLockFile _statusLock;

	qint64 getPid();
};

#endif // STANDARDSERVICECONTROL_H
