#include "%{SvcHdrName}"

%{SvcCn}::%{SvcCn}(int &argc, char **argv) :
	Service{argc, argv}
{
	QCoreApplication::setApplicationName(QStringLiteral(TARGET));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
	//...
}

QtService::Service::CommandResult %{SvcCn}::onStart()
{
@if '%{SocketPort}'
	auto socket = getSocket();
@endif
	return OperationCompleted;
}

QtService::Service::CommandResult %{SvcCn}::onStop(int &exitCode)
{
	exitCode = EXIT_SUCCESS;
	return OperationCompleted;
}
