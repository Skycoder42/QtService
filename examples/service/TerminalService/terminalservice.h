#ifndef TERMINALSERVICE_H
#define TERMINALSERVICE_H

#include <QtService/Service>
#include <QtCore/QCommandLineParser>

class TerminalService : public QtService::Service
{
	Q_OBJECT

public:
	explicit TerminalService(int &argc, char **argv);

	// Service interface
protected:
	CommandResult onStart() override;
	CommandResult onStop(int &exitCode) override;
	bool verifyCommand(const QStringList &arguments) override;

	// Service interface
protected Q_SLOTS:
	void terminalConnected(QtService::Terminal *terminal) override;

private:
	bool parseArguments(QCommandLineParser &parser, const QStringList &arguments);
};

#endif // TERMINALSERVICE_H
