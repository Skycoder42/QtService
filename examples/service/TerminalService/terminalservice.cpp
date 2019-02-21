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

Service::CommandResult TerminalService::onStart()
{
	qDebug() << "Service started with terminal mode:" << terminalMode();
	return OperationCompleted;
}

Service::CommandResult TerminalService::onStop(int &exitCode)
{
	qDebug() << "Closing down service...";
	Q_UNUSED(exitCode)
	return OperationCompleted;
}

bool TerminalService::verifyCommand(const QStringList &arguments)
{
	QCommandLineParser parser;
	if(parseArguments(parser, arguments)) {
		// print help/version if requested. Quits the terminal before even trying to connect
		if(parser.isSet(QStringLiteral("help")))
			parser.showHelp();
		if(parser.isSet(QStringLiteral("version")))
			parser.showVersion();

		if(parser.isSet(QStringLiteral("passive")))
			setTerminalMode(Service::ReadWritePassive);
		return true;
	} else
		return false;
}

void TerminalService::terminalConnected(Terminal *terminal)
{
	qDebug() << "new terminal connected with args:" << terminal->command();
	connect(terminal, &Terminal::terminalDisconnected,
			this, [](){
		qDebug() << "A terminal just disconnected";
	});

	QCommandLineParser parser;
	if(!parseArguments(parser, terminal->command())) {
		terminal->disconnectTerminal();
		return;
	}

	if(parser.positionalArguments().startsWith(QStringLiteral("stop")))
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

bool TerminalService::parseArguments(QCommandLineParser &parser, const QStringList &arguments)
{
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption({
						 {QStringLiteral("p"), QStringLiteral("passive")},
						 QStringLiteral("Run terminal service in passive (Non-Interactive) mode")
					 });
	parser.addPositionalArgument(QStringLiteral("stop"),
								 QStringLiteral("Stop the the service"),
								 QStringLiteral("[stop]"));

	return parser.parse(arguments);
}
