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

class ServicePrivate;
class Q_SERVICE_EXPORT Service : public QObject
{
	Q_OBJECT

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

	static int getSocket();
	static QVector<int> getAllSockets();
	static QHash<int, QByteArray> getAllSocketsNamed();

public Q_SLOTS:
	virtual bool preStart();
	virtual void start() = 0;
	virtual void stop();

	virtual void processCommand(int code);

#ifdef Q_OS_ANDROID
	virtual QAndroidBinder *onBind(const QAndroidIntent &intent);
#endif

protected:
	virtual void pause();
	virtual void resume();
	virtual void reload();

private:
	QScopedPointer<ServicePrivate> d;
};

}

#endif // QTSERVICE_SERVICE_H
