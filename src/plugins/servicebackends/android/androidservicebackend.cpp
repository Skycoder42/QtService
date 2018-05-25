#include "androidservicebackend.h"
#include <QtCore/QEventLoop>
#include <QtAndroidExtras/QAndroidService>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidIntent>
#include <QtAndroidExtras/QAndroidJniExceptionCleaner>
using namespace QtService;

AndroidServiceBackend::AndroidServiceBackend(QObject *parent) :
	ServiceBackend{parent}
{}

int AndroidServiceBackend::runService(Service *service, int &argc, char **argv, int flags)
{
	QAndroidService app(argc, argv,
						std::bind(&ServiceBackend::onBind, service, std::placeholders::_1),
						flags);
	//TODO handle start commands (intents) as well
	_service = service;
	_javaService = QtAndroid::androidService();
	if(!preStartService(_service))
		return EXIT_FAILURE;

	connect(qApp, &QCoreApplication::aboutToQuit,
			this, &AndroidServiceBackend::onExit,
			Qt::DirectConnection);

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::Service*, _service),
							  Q_ARG(int, StartCommand));
	return app.exec();
}

void AndroidServiceBackend::quitService()
{
	//TODO move to java class
	QAndroidJniExceptionCleaner cleaner {QAndroidJniExceptionCleaner::OutputMode::Verbose};
	_javaService.callMethod<void>("stopForeground", "(Z)V",
								  static_cast<jboolean>(true));
	auto svcClass = _javaService.callObjectMethod("getClass", "()Ljava/lang/Class;");
	auto svcName = svcClass.callObjectMethod("getCanonicalName", "()Ljava/lang/String;");
	auto svcNameData = svcName.toString().toUtf8();
	QAndroidIntent intent { _javaService, svcNameData.constData()};
	_javaService.callMethod<jboolean>("stopService", "(Landroid/content/Intent;)Z",
									  intent.handle().object());
}

void AndroidServiceBackend::reloadService()
{
	processServiceCommand(_service, ReloadCommand);
}

void AndroidServiceBackend::onExit()
{
	QEventLoop exitLoop;
	connect(_service, &Service::stopped,
			&exitLoop, &QEventLoop::exit);
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::Service*, _service),
							  Q_ARG(int, StopCommand));
	auto subRes = exitLoop.exec();
	//TODO pass result to java class
}
