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
	enum CommandMode {
		Synchronous,
		Asynchronous
	};
	Q_ENUM(CommandMode)

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
	void started();
	void stopped(int exitCode);
	void reloaded();

protected:
	virtual bool preStart();

	virtual CommandMode onStart() = 0;
	virtual CommandMode onStop(int &exitCode);
	virtual CommandMode onReload();

	virtual void onPause();
	virtual void onResume();

	virtual void onCommand(int code);

#ifdef Q_OS_ANDROID
	virtual void onStartCommand(const QAndroidIntent &intent);
	virtual QAndroidBinder *onBind(const QAndroidIntent &intent);
#endif

private:
	friend class QtService::ServiceBackend;
	QScopedPointer<ServicePrivate> d;
};

}

#endif // QTSERVICE_SERVICE_H
