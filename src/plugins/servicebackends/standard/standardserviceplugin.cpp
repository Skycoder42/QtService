#include "standardserviceplugin.h"
#include "standardservicebackend.h"
#include "standardservicecontrol.h"
#include <QtCore/QStandardPaths>

Q_LOGGING_CATEGORY(logQtService, "qtservice.servicebackends.standard", QtInfoMsg);

StandardServicePlugin::StandardServicePlugin(QObject *parent) :
	QObject{parent}
{}

QString StandardServicePlugin::currentServiceId() const
{
	return QCoreApplication::applicationFilePath();
}

QtService::ServiceBackend *StandardServicePlugin::createServiceBackend(const QString &provider, QtService::Service *service)
{
	if(provider == QStringLiteral("standard"))
		return new StandardServiceBackend{false, service};
	else if(provider == QStringLiteral("debug"))
		return new StandardServiceBackend{true, service};
	else
		return nullptr;
}

QtService::ServiceControl *StandardServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	auto parts = detectNamedService(serviceId);
	if(!parts.first.isEmpty()) {
		// first: search wherever this executable is
		serviceId = QStandardPaths::findExecutable(parts.first, {QCoreApplication::applicationDirPath()});
		// second: search in system paths
		if(serviceId.isEmpty())
			serviceId = QStandardPaths::findExecutable(parts.first);
		// third: just use the name. Will still fail, but at least give the correct error message
		if(serviceId.isEmpty())
			serviceId = parts.first;
	}

	if(backend == QStringLiteral("standard"))
		return new StandardServiceControl{false, std::move(serviceId), parent};
	else if(backend == QStringLiteral("debug"))
		return new StandardServiceControl{true, std::move(serviceId), parent};
	else
		return nullptr;
}
