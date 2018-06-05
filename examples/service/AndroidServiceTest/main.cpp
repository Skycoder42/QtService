#include <QGuiApplication>
#include <QtQml>
#include <QtService/ServiceControl>
#include <QAndroidJniObject>
#include <QtAndroid>
#include "testservice.h"
#include "controlhelper.h"

namespace {

int serviceMain(int argc, char *argv[])
{
	TestService service{argc, argv};
	return service.exec();
}

int activityMain(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication app(argc, argv);

	QAndroidJniObject::callStaticMethod<void>("de/skycoder42/qtservice/test/TestServiceHelper",
											  "registerChannel", "(Landroid/content/Context;)V",
											  QtAndroid::androidContext().object());

	qmlRegisterUncreatableType<QtService::ServiceControl>("de.skycoder42.QtService", 1, 0, "ServiceControl", QStringLiteral("baum"));

	QQmlApplicationEngine engine;
	auto control = QtService::ServiceControl::create(QStringLiteral("android"), QStringLiteral("de.skycoder42.qtservice.AndroidService"), &engine);
	auto helper = new ControlHelper{control};
	engine.rootContext()->setContextProperty(QStringLiteral("control"), control);
	engine.rootContext()->setContextProperty(QStringLiteral("helper"), helper);
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}

}

int main(int argc, char *argv[])
{
	for(auto i = 0; i < argc; i++) {
		if(qstrcmp(argv[i], "--backend") == 0)
			return serviceMain(argc, argv);
	}
	return activityMain(argc, argv);
}
