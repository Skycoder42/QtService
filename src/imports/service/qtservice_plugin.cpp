#include "qtservice_plugin.h"

#include <QtQml>

#include <QtService/ServiceControl>
#include <QtService/Service>
#include <QtService/Terminal>

#include "qmlservicesingleton.h"

namespace {

QObject *createSingleton(QQmlEngine *engine, QJSEngine *)
{
	return new QtService::QmlServiceSingleton(engine);
}

}

QtServiceDeclarativeModule::QtServiceDeclarativeModule(QObject *parent) :
	QQmlExtensionPlugin(parent)
{}

void QtServiceDeclarativeModule::registerTypes(const char *uri)
{
	Q_ASSERT(qstrcmp(uri, "de.skycoder42.QtService") == 0);

	//Version 2.0
	qmlRegisterUncreatableType<QtService::ServiceControl>(uri, 2, 0, "ServiceControl", QStringLiteral("A service cannot be created with parameters. Use the QtService.createControl instead."));
	qmlRegisterUncreatableType<QtService::Service>(uri, 2, 0, "Service", QStringLiteral("A service cannot be created. Use QtService.service instead."));
	qmlRegisterUncreatableType<QtService::Terminal>(uri, 2, 0, "Terminal", QStringLiteral("Terminals cannot be created. They can only be produced by the service itself."));

	qmlRegisterSingletonType<QtService::QmlServiceSingleton>(uri, 2, 0, "QtService", createSingleton);

	// Check to make shure no module update is forgotten
	static_assert(VERSION_MAJOR == 2 && VERSION_MINOR == 0, "QML module version needs to be updated");
}
