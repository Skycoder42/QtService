#include "windowsservicebackend.h"

WindowsServiceBackend::WindowsServiceBackend(QObject *parent) :
	ServiceBackend(parent)
{}

int WindowsServiceBackend::runService(QtService::Service *service, int &argc, char **argv, int flags)
{
	_service = service;
	return EXIT_FAILURE;
}

void WindowsServiceBackend::quitService()
{

}

void WindowsServiceBackend::reloadService()
{
	processServiceCommand(_service, ReloadCommand);
}
