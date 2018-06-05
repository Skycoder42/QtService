#include "testservice.h"
#include <QtDebug>
#include <QTimer>
#include <QtAndroid>
#include <QAtomicInt>
#include <QAndroidParcel>

namespace {

QAtomicInt wasStarted = 0;

}

TestService::TestService(int &argc, char **argv) :
	Service{argc, argv}
{
	addCallback("onStartCommand", &TestService::onStartCommand);
	addCallback("onBind", &TestService::onBind);
}

QtService::Service::CommandMode TestService::onStart()
{
	qDebug() << "onStart";
	doStartNotify();
	//QTimer::singleShot(5000, this, &TestService::quit);
	return Synchronous;
}

QtService::Service::CommandMode TestService::onStop(int &)
{
	qDebug() << "onStop";
	QAndroidJniObject::callStaticMethod<void>("de/skycoder42/qtservice/test/TestServiceHelper",
											  "stopNotifyRunning", "(Landroid/app/Service;)V",
											  QtAndroid::androidService().object());
//	QTimer::singleShot(3000, this, [this](){
//		emit stopped();
//	});
//	return Asynchronous;
	return Synchronous;
}

int TestService::onStartCommand(const QAndroidIntent &intent, int flags, int startId)
{
	qDebug() << "onStartCommand" << intent.handle().toString() << flags << startId;
	doStartNotify();
	auto action = intent.handle().callObjectMethod("getAction", "()Ljava/lang/String;").toString();
	if(!action.isEmpty()) {
		QAndroidJniObject::callStaticMethod<void>("de/skycoder42/qtservice/test/TestServiceHelper",
												  "updateNotifyRunning", "(Landroid/app/Service;Ljava/lang/String;)V",
												  QtAndroid::androidService().object(),
												  QAndroidJniObject::fromString(QStringLiteral("Service intent with action: ") + action).object());
	}
	return 1; // START_STICKY
}

QAndroidBinder *TestService::onBind(const QAndroidIntent &intent)
{
	qDebug() << "onBind" << intent.handle().toString();
	return new QAndroidBinder{};
}

void TestService::doStartNotify()
{
	if(wasStarted.testAndSetOrdered(0, 1)) {
		QAndroidJniObject::callStaticMethod<void>("de/skycoder42/qtservice/test/TestServiceHelper",
												  "notifyRunning", "(Landroid/app/Service;Ljava/lang/String;)V",
												  QtAndroid::androidService().object(),
												  QAndroidJniObject::fromString(QStringLiteral("Service started…")).object());
	}
}
