#ifndef STANDARDSERVICECONTROL_H
#define STANDARDSERVICECONTROL_H

#include <QtCore/QLockFile>
#include <QtService/ServiceControl>

class StandardServiceControl : public QtService::ServiceControl
{
	Q_OBJECT

public:
	explicit StandardServiceControl(QString &&serviceId, QObject *parent = nullptr);

	SupportFlags supportFlags() const override;
	ServiceStatus status() const override;
	QString backend() const override;

public Q_SLOTS:
	bool start() override;
	bool stop() override;

protected:
	QString serviceName() const override;

private:
	mutable QLockFile _statusLock;

	qint64 getPid();
};

#endif // STANDARDSERVICECONTROL_H
