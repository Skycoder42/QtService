#ifndef QTSERVICE_SERVICE_H
#define QTSERVICE_SERVICE_H

#include <QtCore/qobject.h>

namespace QtService {

class Service : public QObject
{
	Q_OBJECT

public:
	explicit Service(QObject *parent = nullptr);

};

}

#endif // QTSERVICE_SERVICE_H
