#include "echoservice.h"

int main(int argc, char *argv[])
{
	EchoService svc(argc, argv);
	return svc.exec();
}
