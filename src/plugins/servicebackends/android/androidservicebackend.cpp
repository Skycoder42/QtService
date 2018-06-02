#include "androidservicebackend.h"
#include "androidserviceplugin.h"
#include <QtCore/QEventLoop>
#include <QtAndroidExtras/QAndroidService>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniExceptionCleaner>
using namespace QtService;

AndroidServiceBackend::AndroidServiceBackend(Service *service) :
	ServiceBackend{service}
{}

int AndroidServiceBackend::runService(int &argc, char **argv, int flags)
{
	QAndroidService app(argc, argv,
						std::bind(&AndroidServiceBackend::onBind, this, std::placeholders::_1),
						flags);
	//TODO handle onStartCommand intents? -> copy from onbind
	_javaService = QtAndroid::androidService();
	if(!preStartService())
		return EXIT_FAILURE;

	connect(qApp, &QCoreApplication::aboutToQuit,
			this, &AndroidServiceBackend::onExit,
			Qt::DirectConnection);

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, StartCommand));
	return app.exec();
}

void AndroidServiceBackend::quitService()
{
	QAndroidJniExceptionCleaner cleaner {QAndroidJniExceptionCleaner::OutputMode::Verbose};
	_javaService.callMethod<void>("stopSelf");
}

void AndroidServiceBackend::reloadService()
{
	processServiceCommand(ReloadCommand);
}

void AndroidServiceBackend::onExit()
{
	QAndroidJniExceptionCleaner cleaner {QAndroidJniExceptionCleaner::OutputMode::Verbose};
	QEventLoop exitLoop;
	connect(service(), &Service::stopped,
			&exitLoop, &QEventLoop::exit);
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, StopCommand));
	auto subRes = exitLoop.exec();
	_javaService.setField<jint>("_exitCode", subRes);
}

QAndroidBinder *AndroidServiceBackend::onBind(const QAndroidIntent &intent)
{
	return processServiceCallback<QAndroidBinder*>("onBind", intent);
}
