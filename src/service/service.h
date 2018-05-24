#ifndef QTSERVICE_SERVICE_H
#define QTSERVICE_SERVICE_H

#include <QtCore/qobject.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qvector.h>
#include <QtCore/qhash.h>

#include "QtService/qtservice_global.h"

#ifdef Q_OS_ANDROID
class QAndroidBinder;
class QAndroidIntent;
#endif

namespace QtService {

class ServiceBackend;
class ServicePrivate;
class Q_SERVICE_EXPORT Service : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString backend READ backend CONSTANT)

public:
	enum StandardCode {
		PauseCode,
		ResumeCode,
		ReloadCode
	};
	Q_ENUM(StandardCode)

	explicit Service(int &argc, char **argv, int = QCoreApplication::ApplicationFlags);
	~Service() override;

	int exec();

	static Service *instance();

	int getSocket();
	QList<int> getAllSockets();
	QHash<int, QByteArray> getAllSocketsNamed();

	QString backend() const;

public Q_SLOTS:
	void quit();
	void reload();

Q_SIGNALS:
	void stopped(int exitCode, QPrivateSignal);

protected Q_SLOTS:
	void stopCompleted(int exitCode = EXIT_SUCCESS);

protected:
	virtual bool preStart();
	virtual void onStart() = 0; //TODO make async?
	virtual void onStop();

	virtual void onPause();
	virtual void onResume();
	virtual void onReload(); //TODO make async?

	virtual void onCommand(int code);

#ifdef Q_OS_ANDROID
	virtual QAndroidBinder *onBind(const QAndroidIntent &intent);
#endif

private:
	friend class QtService::ServiceBackend;
	QScopedPointer<ServicePrivate> d;
};

}

#endif // QTSERVICE_SERVICE_H
