#ifndef TESTSERVICE_H
#define TESTSERVICE_H

#include <QtService/Service>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QTcpServer>
#include <QtCore/QDataStream>

class TestService : public QtService::Service
{
	Q_OBJECT

public:
	explicit TestService(int &argc, char **argv);

protected:
	bool preStart() override;
	CommandResult onStart() override;
	CommandResult onStop(int &exitCode) override;
	CommandResult onReload() override;
	CommandResult onPause() override;
	CommandResult onResume() override;

	QVariant onCallback(const QByteArray &kind, const QVariantList &args) override;

	bool verifyCommand(const QStringList &arguments) override;

protected Q_SLOTS:
	void terminalConnected(QtService::Terminal *terminal) override;

private:
	QLocalServer *_server = nullptr;
	QLocalSocket *_socket = nullptr;
	QDataStream _stream;

	QTcpServer *_activatedServer = nullptr;
};

#endif // TESTSERVICE_H
