#ifndef QMLSERVICESINGLETON_H
#define QMLSERVICESINGLETON_H

#include <QObject>
#include <QtService/Service>
#include <QtService/ServiceControl>

#ifdef DOXYGEN_RUN
namespace de::skycoder42::QtService {

/*! @brief A QML singleton to create service controls and access the service instance
 *
 * @extends QtQml.QtObject
 * @since 1.0
 *
 * @sa QtService::Service, QtService::ServiceControl
 */
class QtService
#else
namespace QtService {

class QmlServiceSingleton : public QObject
#endif
{
	Q_OBJECT

	/*! @brief A reference to the current service instance, if accessed from within a service
	 *
	 * @default{`nullptr`}
	 *
	 * If you are using qml in a service, you can use this property to get a reference to the
	 * current service instance. If not used from within a service, nullptr is returned.
	 *
	 * @accessors{
	 *	@memberAc{service}
	 *  @readonlyAc
	 *  @constantAc
	 * }
	 *
	 * @sa QtService::Service
	 */
	Q_PROPERTY(QtService::Service* service READ service CONSTANT)

public:
	//! @private
	explicit QmlServiceSingleton(QObject *parent = nullptr);

	//! @copydoc QtService::ServiceControl::create
	Q_INVOKABLE QtService::ServiceControl *createControl(const QString &backend, QString serviceId, QObject *parent = nullptr) const;
	//! @copydoc QtService::ServiceControl::createFromName(const QString &, const QString &, QObject *)
	Q_INVOKABLE QtService::ServiceControl *createControlFromName(const QString &backend, const QString &serviceName, QObject *parent = nullptr) const;
	//! @copydoc QtService::ServiceControl::createFromName(const QString &, const QString &, const QString &, QObject *)
	Q_INVOKABLE QtService::ServiceControl *createControlFromName(const QString &backend, const QString &serviceName, const QString &domain, QObject *parent = nullptr) const;

	//! @private
	QtService::Service* service() const;
};

}

#endif // QMLSERVICESINGLETON_H
