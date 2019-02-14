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
		{"callStartCommand", "(Landroid/content/Intent;III)I", reinterpret_cast<void*>(&AndroidServiceBackend::callStartCommand)},
		{"exitService", "()Z", reinterpret_cast<void*>(&AndroidServiceBackend::exitService)}
	};
	env->RegisterNatives(env->FindClass("de/skycoder42/qtservice/AndroidService"),
						 methods,
						 sizeof(methods)/sizeof(JNINativeMethod));
	_javaService.callMethod<void>("nativeReady");

	// handle start result
	connect(service(), QOverload<bool>::of(&Service::started),
			this, &AndroidServiceBackend::onStarted);

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

jint AndroidServiceBackend::callStartCommand(JNIEnv *, jobject, jobject intent, jint flags, jint startId, jint oldId)
{
	if (_backendInstance) {
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

jboolean AndroidServiceBackend::exitService(JNIEnv *, jobject)
{
	if (_backendInstance)
		return QMetaObject::invokeMethod(_backendInstance, "onExit", Qt::QueuedConnection);
	else
		return false;
}

void AndroidServiceBackend::onStarted(bool success)
{
	if (!success) {
		_startupFailed = true;
		quitService();
	}
}

void AndroidServiceBackend::onExit()
{
	if (_startupFailed)
		onStopped(EXIT_FAILURE);
	else {
		connect(service(), &Service::stopped,
				this, &AndroidServiceBackend::onStopped,
				Qt::UniqueConnection);
		processServiceCommand(StopCommand);
	}
}

void AndroidServiceBackend::onStopped(int exitCode)
{
	Q_UNUSED(exitCode)  //TODO print result or so
	_javaService.callMethod<void>("nativeExited");
}

QAndroidBinder *AndroidServiceBackend::onBind(const QAndroidIntent &intent)
{
	return processServiceCallback<QAndroidBinder*>("onBind", intent);
}
