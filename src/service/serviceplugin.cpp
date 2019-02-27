#include "serviceplugin.h"
#include <QtCore/QRegularExpression>
#include <QtCore/QCoreApplication>

QtService::ServicePlugin::ServicePlugin() = default;

QtService::ServicePlugin::~ServicePlugin() = default;

QString QtService::ServicePlugin::currentServiceId(const QString &backend) const
{
	return findServiceId(backend, QCoreApplication::applicationName(), QCoreApplication::organizationDomain());
}
