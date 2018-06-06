#include "androidservicebackend.h"
#include "androidserviceplugin.h"
#include <QtCore/QEventLoop>
#include <QtAndroidExtras/QAndroidService>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniExceptionCleaner>
#include <QPointer>
using namespace QtService;

QPointer<AndroidServiceBackend> AndroidServiceBackend::_backendInstance = nullptr;

AndroidServiceBackend::AndroidServiceBackend(Service *service) :
	ServiceBackend{service}
{
	_backendInstance = this;
}

int AndroidServiceBackend::runService(int &argc, char **argv, int flags)
{
	QAndroidService app(argc, argv,
						std::bind(&AndroidServiceBackend::onBind, this, std::placeholders::_1),
						flags);
	if(!preStartService())
		return EXIT_FAILURE;

	//NOTE check if onStartCommand is supported with QAndroidService yet
	_javaService = QtAndroid::androidService();
	QAndroidJniEnvironment env;
	static const JNINativeMethod methods[] = {
		{"callStartCommand", "(Landroid/content/Intent;III)I", reinterpret_cast<void*>(callStartCommand)}
	};
	env->RegisterNatives(env->FindClass("de/skycoder42/qtservice/AndroidService"), methods, 1);
	_javaService.callMethod<void>("nativeReady");

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

jint AndroidServiceBackend::onStartCommand(jobject intent, jint flags, jint startId, jint oldId)
{
	if(_backendInstance) {
		auto var = _backendInstance->processServiceCallbackImpl("onStartCommand", QVariantList {
																	QVariant::fromValue(QAndroidIntent{intent}),
																	static_cast<int>(flags),
																	static_cast<int>(startId)
																});
		auto ok = false;
		auto res = var.toInt(&ok);
		if(ok)
			return res;
	}

	return oldId;
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
}

QAndroidBinder *AndroidServiceBackend::onBind(const QAndroidIntent &intent)
{
	return processServiceCallback<QAndroidBinder*>("onBind", intent);
}

jint JNICALL callStartCommand(JNIEnv*, jobject, jobject intent, jint flags, jint startId, jint oldId)
{
	return AndroidServiceBackend::onStartCommand(intent, flags, startId, oldId);
}
