#include <QCoreApplication>
#include "testservice.h"

int main(int argc, char *argv[])
{
	TestService service{argc, argv};
	QCoreApplication::setApplicationName(QStringLiteral("testservice"));
	QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
	return service.exec();
}
