#include "echoservice.h"

int main(int argc, char *argv[])
{
	EchoService svc(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("echoservice"));
	QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
	//...
	return svc.exec();
}
