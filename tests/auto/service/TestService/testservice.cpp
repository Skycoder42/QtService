#include "testservice.h"

#include <QTimer>
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
	_server = new QLocalServer(this);
	_server->setSocketOptions(QLocalServer::WorldAccessOption);
	connect(_server, &QLocalServer::newConnection, this, [this](){
		if(_socket)
			return;
		_socket = _server->nextPendingConnection();
		_stream.setDevice(_socket);
		_server->close();

		emit started();
		_stream << QByteArray("started");
	});
	_server->listen(runtimeDir().absoluteFilePath(QStringLiteral("__qtservice_testservice")));
	QTimer::singleShot(10000, this, [this](){
		if(!_socket)
			qApp->quit();
	});
	return Asynchronous;
}

Service::CommandMode TestService::onStop(int &exitCode)
{
	_stream << QByteArray("stopping");
	exitCode = _socket->waitForBytesWritten(5000);
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
