#include "terminalservice.h"
#include <QtService/Terminal>
#include <QtCore/QDebug>
using namespace QtService;

TerminalService::TerminalService(int &argc, char **argv) :
	Service(argc, argv)
{
	setTerminalActive(true);
	setStartWithTerminal(true);
	//setTerminalMode(Service::ReadWritePassive);
}

Service::CommandMode TerminalService::onStart()
{
	qDebug() << "Service started with terminal mode:" << terminalMode();
	return Synchronous;
}

Service::CommandMode TerminalService::onStop(int &exitCode)
{
	qDebug() << "Closing down service...";
	Q_UNUSED(exitCode)
	return Synchronous;
}

bool TerminalService::verifyCommand(const QStringList &arguments)
{
	if(arguments.contains(QStringLiteral("--passive")))
		setTerminalMode(Service::ReadWritePassive);
	return true;
}

void TerminalService::terminalConnected(Terminal *terminal)
{
	qDebug() << "new terminal connected with args:" << terminal->command();
	connect(terminal, &Terminal::terminalDisconnected,
			this, [](){
		qDebug() << "A terminal just disconnected";
	});

	if(terminal->command().startsWith(QStringLiteral("stop")))
		quit();
	else if(terminal->terminalMode() == Service::ReadWriteActive) {
		connect(terminal, &Terminal::readyRead,
				terminal, [terminal](){
			qDebug() << "terminals name is:" << terminal->readAll();
			terminal->disconnectTerminal();
		});
		terminal->write("Please enter your name: ");
		terminal->requestLine();
	} else {
		connect(terminal, &Terminal::readyRead,
				terminal, [terminal](){
			auto data = terminal->readAll();
			qDebug() << "teminal said:" << data;
			terminal->write(data);
		});
	}
}
