#include "testservice.h"
using namespace QtService;

TestService::TestService(int &argc, char **argv) :
	Service{argc, argv}
{}

bool TestService::preStart()
{
	return true;
}

Service::CommandMode TestService::onStart()
{
	return Synchronous;
}

Service::CommandMode TestService::onStop(int &exitCode)
{
	return Synchronous;
}

Service::CommandMode TestService::onReload()
{
	return Synchronous;
}

Service::CommandMode TestService::onPause()
{
	return Synchronous;
}

Service::CommandMode TestService::onResume()
{
	return Synchronous;
}

QVariant TestService::onCallback(const QByteArray &kind, const QVariantList &args)
{
	return {};
}
