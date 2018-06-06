#ifndef QMLSERVICESINGLETON_H
#define QMLSERVICESINGLETON_H

#include <QObject>
#include <QtService/Service>
#include <QtService/ServiceControl>

namespace QtService {

class QmlServiceSingleton : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QtService::Service* service READ service CONSTANT)

public:
	explicit QmlServiceSingleton(QObject *parent = nullptr);

	Q_INVOKABLE QtService::ServiceControl *createControl(const QString &backend, QString serviceId, QObject *parent = nullptr) const;

	QtService::Service* service() const;
};

}

#endif // QMLSERVICESINGLETON_H
