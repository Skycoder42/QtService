#include "standardserviceplugin.h"
#include "standardservicebackend.h"
#include "standardservicecontrol.h"
#include <QtCore/QStandardPaths>

StandardServicePlugin::StandardServicePlugin(QObject *parent) :
	QObject{parent}
{}

QString StandardServicePlugin::currentServiceId(const QString &backend) const
{
	if (backend == QStringLiteral("standard") ||
		backend == QStringLiteral("debug"))
		return QCoreApplication::applicationFilePath();
	else
		return {};
}

QString StandardServicePlugin::findServiceId(const QString &backend, const QString &serviceName, const QString &domain) const
{
	Q_UNUSED(domain)

	if (backend == QStringLiteral("standard") ||
		backend == QStringLiteral("debug")) {
		// first: search wherever this executable is
		auto serviceId = QStandardPaths::findExecutable(serviceName, {QCoreApplication::applicationDirPath()});
		// second: search in system paths
		if (serviceId.isEmpty())
			serviceId = QStandardPaths::findExecutable(serviceName);
		// if found, return the result
		if (!serviceId.isEmpty())
			return serviceId;
	}

	return {};
}

QtService::ServiceBackend *StandardServicePlugin::createServiceBackend(const QString &backend, QtService::Service *service)
{
	if (backend == QStringLiteral("standard"))
		return new StandardServiceBackend{false, service};
	else if (backend == QStringLiteral("debug"))
		return new StandardServiceBackend{true, service};
	else
		return nullptr;
}

QtService::ServiceControl *StandardServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	if (backend == QStringLiteral("standard"))
		return new StandardServiceControl{false, std::move(serviceId), parent};
	else if (backend == QStringLiteral("debug"))
		return new StandardServiceControl{true, std::move(serviceId), parent};
	else
		return nullptr;
}
