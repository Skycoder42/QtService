#ifndef ANDROIDSERVICEBACKEND_H
#define ANDROIDSERVICEBACKEND_H

#include <QtService/ServiceBackend>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QAndroidBinder>
#include <QtAndroidExtras/QAndroidIntent>

class AndroidServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	explicit AndroidServiceBackend(QtService::Service *service);

	int runService(int &argc, char **argv, int flags) override;
	void quitService() override;
	void reloadService() override;

	// helper stuff
	static jint onStartCommand(jobject intent, jint flags, jint startId, jint oldId);

private Q_SLOTS:
	void onStarted(bool success);
	void onExit();

private:
	static QPointer<AndroidServiceBackend> _backendInstance;
	QAndroidJniObject _javaService;

	QAndroidBinder* onBind(const QAndroidIntent &intent);
};

jint JNICALL callStartCommand(JNIEnv *env, jobject object, jobject intent, jint flags, jint startId, jint oldId);

#endif // ANDROIDSERVICEBACKEND_H
