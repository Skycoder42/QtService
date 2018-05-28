#include <QCoreApplication>
#include "testservice.h"

int main(int argc, char *argv[])
{
	TestService service{argc, argv};
	return service.exec();
}
