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
	return SupportsStartStop |
			SupportsCustomCommands |
			SupportsNonBlocking |
			SupportsDisable;
}

bool AndroidServiceControl::serviceExists() const
{
	const auto svcInfo = serviceInfo();
	QAndroidJniExceptionCleaner cleaner;
	return svcInfo.isValid() &&
			!svcInfo.getObjectField("name", "Ljava/lang/String;").toString().isEmpty();
}

bool AndroidServiceControl::isEnabled() const
{
	auto componentName = serviceComponent();
	if(!componentName.isValid())
		return false;

	QAndroidJniExceptionCleaner cleaner;
	static const auto COMPONENT_ENABLED_STATE_DEFAULT = QAndroidJniObject::getStaticField<jint>("android/content/pm/PackageManager", "COMPONENT_ENABLED_STATE_DEFAULT");
	static const auto COMPONENT_ENABLED_STATE_ENABLED = QAndroidJniObject::getStaticField<jint>("android/content/pm/PackageManager", "COMPONENT_ENABLED_STATE_ENABLED");

	auto pm = QtAndroid::androidContext().callObjectMethod("getPackageManager", "()Landroid/content/pm/PackageManager;");
	const auto componentState = pm.callMethod<jint>("getComponentEnabledSettin", "(Landroid/content/ComponentName;)I",
													componentName.object());

	if (componentState == COMPONENT_ENABLED_STATE_ENABLED)
		return true;
	else if(componentState != COMPONENT_ENABLED_STATE_DEFAULT)
		return false;
	else {
		// special case: was never changes, so value from manifest is needed
		auto svcInfo = serviceInfo();
		if(svcInfo.isValid())
			return svcInfo.callMethod<jboolean>("isEnabled");
		else
			return false;
	}
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

bool AndroidServiceControl::setEnabled(bool enabled)
{
	auto componentName = serviceComponent();
	if(!componentName.isValid())
		return false;

	QAndroidJniEnvironment env;
	QAndroidJniExceptionCleaner cleaner;
	static const auto COMPONENT_ENABLED_STATE_ENABLED = QAndroidJniObject::getStaticField<jint>("android/content/pm/PackageManager", "COMPONENT_ENABLED_STATE_ENABLED");
	static const auto COMPONENT_ENABLED_STATE_DISABLED = QAndroidJniObject::getStaticField<jint>("android/content/pm/PackageManager", "COMPONENT_ENABLED_STATE_DISABLED");

	auto pm = QtAndroid::androidContext().callObjectMethod("getPackageManager", "()Landroid/content/pm/PackageManager;");
	pm.callMethod<void>("setComponentEnabledSetting", "(Landroid/content/ComponentName;II)V",
						componentName.object(),
						enabled ? COMPONENT_ENABLED_STATE_ENABLED : COMPONENT_ENABLED_STATE_DISABLED,
						0);
	return !env->ExceptionCheck();
}

QString AndroidServiceControl::serviceName() const
{
	return serviceId().split(QLatin1Char('.')).last();
}

QByteArray AndroidServiceControl::jniServiceId() const
{
	return serviceId().replace(QLatin1Char('.'), QLatin1Char('/')).toUtf8();
}

QAndroidJniObject AndroidServiceControl::serviceComponent() const
{
	QAndroidJniExceptionCleaner cleaner;
	QAndroidJniEnvironment env;

	QAndroidJniObject jClass{static_cast<jobject>(env->FindClass(jniServiceId().constData()))};
	if(!jClass.isValid())
		return {};

	QAndroidJniObject componentName {
		"android/content/ComponentName", "(Landroid/content/Context;Ljava/lang/Class;)V",
		QtAndroid::androidContext().object(),
		jClass.object()
	};
	if(!componentName.isValid())
		return {};
	else
		return componentName;
}

QAndroidJniObject AndroidServiceControl::serviceInfo() const
{
	const auto componentName = serviceComponent();
	if(!componentName.isValid())
		return {};

	QAndroidJniExceptionCleaner cleaner;
	static const auto MATCH_DISABLED_COMPONENTS = QAndroidJniObject::getStaticField<jint>("android/content/pm/PackageManager", "MATCH_DISABLED_COMPONENTS");
	static const auto MATCH_DIRECT_BOOT_AWARE = QAndroidJniObject::getStaticField<jint>("android/content/pm/PackageManager", "MATCH_DIRECT_BOOT_AWARE");
	static const auto MATCH_DIRECT_BOOT_UNAWARE = QAndroidJniObject::getStaticField<jint>("android/content/pm/PackageManager", "MATCH_DIRECT_BOOT_UNAWARE");

	const auto pm = QtAndroid::androidContext().callObjectMethod("getPackageManager", "()Landroid/content/pm/PackageManager;");
	const auto svcInfo = pm.callObjectMethod("getServiceInfo", "(Landroid/content/ComponentName;I)Landroid/content/pm/ServiceInfo;",
											 componentName.object(),
											 MATCH_DISABLED_COMPONENTS | MATCH_DIRECT_BOOT_AWARE | MATCH_DIRECT_BOOT_UNAWARE);
	if(svcInfo.isValid())
		return svcInfo;
	else
		return {};
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
									 env->FindClass(jniServiceId().constData()));
	context.callObjectMethod("startService", "(Landroid/content/Intent;)Landroid/content/ComponentName;",
							 intent.handle().object());
}
