#include "echoservice.h"
#include <QStandardPaths>

int main(int argc, char *argv[])
{
	EchoService svc(argc, argv);
	return svc.exec();
}
