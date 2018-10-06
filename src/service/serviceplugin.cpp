#include "serviceplugin.h"
#include <QtCore/QRegularExpression>

QtService::ServicePlugin::ServicePlugin() = default;

QtService::ServicePlugin::~ServicePlugin() = default;

QPair<QString, QString> QtService::ServicePlugin::detectNamedService(const QString &serviceId) const
{
	QRegularExpression regex{QStringLiteral(R"__(^<<(.*)\*(.*?)\.?>>$)__")};
	auto match = regex.match(serviceId);
	if(match.hasMatch())
		return {match.captured(1), match.captured(2)};
	else
		return {};
}
