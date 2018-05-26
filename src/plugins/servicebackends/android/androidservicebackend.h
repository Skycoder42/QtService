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
	explicit AndroidServiceBackend(QObject *parent = nullptr);

	int runService(QtService::Service *service, int &argc, char **argv, int flags) override;
	void quitService() override;
	void reloadService() override;

private Q_SLOTS:
	void onExit();

private:
	QtService::Service *_service = nullptr;
	QAndroidJniObject _javaService;

	QAndroidBinder* onBind(const QAndroidIntent &intent);
};

Q_DECLARE_METATYPE(QAndroidBinder*)
Q_DECLARE_METATYPE(QAndroidIntent)

#endif // ANDROIDSERVICEBACKEND_H
