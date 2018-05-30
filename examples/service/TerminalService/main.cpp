#include "terminalservice.h"

int main(int argc, char *argv[])
{
	TerminalService service(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("terminalservice"));
	QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
	//...
	return service.exec();
}
