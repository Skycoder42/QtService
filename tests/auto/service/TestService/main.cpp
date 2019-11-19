#include <QCoreApplication>
#include "testservice.h"

int main(int argc, char *argv[])
{
	qDebug() << QCoreApplication::libraryPaths();
	TestService service{argc, argv};
	QCoreApplication::setApplicationName(QStringLiteral("testservice"));
	QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("de.skycoder42.qtservice.tests"));
	return service.exec();
}
