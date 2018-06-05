#include "controlhelper.h"
#include <QAndroidIntent>
#include <QtAndroid>

Q_DECLARE_METATYPE(QAndroidIntent)
Q_DECLARE_METATYPE(QAndroidServiceConnection*)
Q_DECLARE_METATYPE(QtAndroid::BindFlags)

ControlHelper::ControlHelper(QtService::ServiceControl *parent) :
	QObject(parent),
	_control(parent)
{}

void ControlHelper::startIntent(const QString &action)
{
	_control->callCommand("startWithIntent", QAndroidIntent{action});
}

void ControlHelper::bind()
{
	if(_connection)
		return;

	_connection = new Connection();
	_control->callCommand("bind",
						  static_cast<QAndroidServiceConnection*>(_connection),
						  QtAndroid::BindFlags{QtAndroid::BindFlag::AutoCreate});
}

void ControlHelper::unbind()
{
	if(!_connection)
		return;
	_control->callCommand("unbind", static_cast<QAndroidServiceConnection*>(_connection));
	_connection = nullptr;
}

void ControlHelper::Connection::onServiceConnected(const QString &name, const QAndroidBinder &serviceBinder)
{
	Q_UNUSED(serviceBinder)
	toast(QStringLiteral("Service bound: ") + name);
}

void ControlHelper::Connection::onServiceDisconnected(const QString &name)
{
	toast(QStringLiteral("Service unbound: ") + name);
}

void ControlHelper::Connection::toast(const QString &message)
{
	QtAndroid::runOnAndroidThread([message](){
		auto toast = QAndroidJniObject::callStaticObjectMethod("android/widget/Toast",
															   "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;",
															   QtAndroid::androidContext().object(),
															   QAndroidJniObject::fromString(message).object(),
															   1);
		toast.callMethod<void>("show");
	});
}
