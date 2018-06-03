#include "androidservicecontrol.h"
#include "androidserviceplugin.h"
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniExceptionCleaner>
using namespace QtService;

AndroidServiceControl::AndroidServiceControl(QString &&serviceId, QObject *parent) :
	ServiceControl{std::move(serviceId), parent}
{}

QString AndroidServiceControl::backend() const
{
	return QStringLiteral("android");
}

ServiceControl::SupportFlags AndroidServiceControl::supportFlags() const
{
	return SupportsStartStop | SupportsCustomCommands | SupportsNonBlocking;
}

bool AndroidServiceControl::serviceExists() const
{
	QAndroidJniExceptionCleaner cleaner;
	QAndroidJniEnvironment env;
	return QAndroidJniObject{static_cast<jobject>(env->FindClass(qUtf8Printable(serviceId())))}.isValid();
}

QVariant AndroidServiceControl::callGenericCommand(const QByteArray &kind, const QVariantList &args)
{
	if(kind == "bind") {
		if(args.size() < 1 || args.size() > 2) {
			setError(tr("The bind command must be called with a QAndroidServiceConnection* as first parameter and QtAndroid::BindFlags as optional second parameter"));
			return {};
		}

		auto connection = args.value(0).value<QAndroidServiceConnection*>();
		if(!connection) {
			setError(tr("The bind command must be called with a QAndroidServiceConnection* as first parameter and QtAndroid::BindFlags as optional second parameter"));
			return {};
		}
		auto flags = args.size() == 2 ? args.value(1).value<QtAndroid::BindFlags>() : QtAndroid::BindFlag::None;

		return bind(connection, flags);
	} else if(kind == "unbind") {
		if(args.size() != 1) {
			setError(tr("The unbind command must be called with a QAndroidServiceConnection* as only parameter"));
			return {};
		}

		auto connection = args.value(0).value<QAndroidServiceConnection*>();
		if(!connection) {
			setError(tr("The unbind command must be called with a QAndroidServiceConnection* as only parameter"));
			return {};
		}

		unbind(connection);
		return {};
	} else if(kind == "startWithIntent") {
		if(args.size() != 1) {
			setError(tr("The startWithIntent command must be called with a QAndroidIntent as only parameter"));
			return {};
		}

		auto intent = args.value(0).value<QAndroidIntent>();
		if(!intent.handle().isValid()) {
			setError(tr("The startWithIntent command must be called with a QAndroidIntent as only parameter"));
			return {};
		}

		startWithIntent(intent);
		return {};
	} else
		return ServiceControl::callGenericCommand(kind, args);
}

bool AndroidServiceControl::start()
{
	if(!serviceExists())
		return false;

	QAndroidJniExceptionCleaner cleaner;
	auto context = QtAndroid::androidContext();
	QAndroidIntent intent{context, qUtf8Printable(serviceId())};
	context.callObjectMethod("startService", "(Landroid/content/Intent;)Landroid/content/ComponentName;",
							 intent.handle().object());
	return true;
}

bool AndroidServiceControl::stop()
{
	if(!serviceExists())
		return false;

	QAndroidJniExceptionCleaner cleaner;
	auto context = QtAndroid::androidContext();
	QAndroidIntent intent{context, qUtf8Printable(serviceId())};
	return context.callMethod<jboolean>("stopService", "(Landroid/content/Intent;)Z",
										intent.handle().object());
}

QString AndroidServiceControl::serviceName() const
{
	return serviceId().split(QLatin1Char('/')).last();
}

bool AndroidServiceControl::bind(QAndroidServiceConnection *serviceConnection, QtAndroid::BindFlags flags)
{
	if(!serviceExists())
		return false;

	QAndroidJniExceptionCleaner cleaner;
	auto context = QtAndroid::androidContext();
	QAndroidIntent intent{context, qUtf8Printable(serviceId())};
	return QtAndroid::bindService(intent, *serviceConnection, flags);
}

void AndroidServiceControl::unbind(QAndroidServiceConnection *serviceConnection)
{
	QAndroidJniExceptionCleaner cleaner;
	auto context = QtAndroid::androidContext();
	context.callMethod<void>("unbindService", "(Landroid/content/ServiceConnection;)V",
							 serviceConnection->handle().object());
}

void AndroidServiceControl::startWithIntent(const QAndroidIntent &intent)
{
	QAndroidJniExceptionCleaner cleaner;
	QAndroidJniEnvironment env;
	auto context = QtAndroid::androidContext();
	intent.handle().callObjectMethod("setClass", "(Landroid/content/Context;Ljava/lang/Class;)Landroid/content/Intent;",
									 context.object(),
									 env->FindClass(qUtf8Printable(serviceId())));
	context.callObjectMethod("startService", "(Landroid/content/Intent;)Landroid/content/ComponentName;",
							 intent.handle().object());
}
