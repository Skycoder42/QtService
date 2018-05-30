#ifndef TERMINALSERVICE_H
#define TERMINALSERVICE_H

#include <QtService/Service>

class TerminalService : public QtService::Service
{
	Q_OBJECT

public:
	explicit TerminalService(int &argc, char **argv);

	// Service interface
protected:
	CommandMode onStart() override;
	CommandMode onStop(int &exitCode) override;

	// Service interface
protected Q_SLOTS:
	void terminalConnected(QtService::Terminal *terminal) override;
};

#endif // TERMINALSERVICE_H
