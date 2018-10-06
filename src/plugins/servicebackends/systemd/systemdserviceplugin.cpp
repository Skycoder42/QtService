#include "systemdserviceplugin.h"
#include "systemdservicebackend.h"
#include "systemdservicecontrol.h"
using namespace QtService;

Q_LOGGING_CATEGORY(logQtService, "qtservice.servicebackends.systemd"); //TODO, QtInfoMsg);

SystemdServicePlugin::SystemdServicePlugin(QObject *parent) :
	QObject(parent)
{}

QString SystemdServicePlugin::currentServiceId() const
{
	return QCoreApplication::applicationName() + QStringLiteral(".service");
}

ServiceBackend *SystemdServicePlugin::createServiceBackend(const QString &provider, Service *service)
{
	if(provider == QStringLiteral("systemd"))
		return new SystemdServiceBackend{service};
	else
		return nullptr;
}

ServiceControl *SystemdServicePlugin::createServiceControl(const QString &backend, QString &&serviceId, QObject *parent)
{
	auto parts = detectNamedService(serviceId);
	if(!parts.first.isEmpty()) {
		// first: get the service control for the long version and create a control instance via recursion
		serviceId = parts.second + QLatin1Char('.') + parts.first + QStringLiteral(".service");
		auto longCtrl = createServiceControl(backend, std::move(serviceId), parent);
		// test if that instance actually exists
		if(longCtrl) {
			if(longCtrl->serviceExists())
				return longCtrl;
			else
				delete longCtrl;
		}

		// second: use the short name and continue as usual
		serviceId = parts.first + QStringLiteral(".service");
	}

	if(backend == QStringLiteral("systemd"))
		return new SystemdServiceControl{std::move(serviceId), parent};
	else
		return nullptr;
}
