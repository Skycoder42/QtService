#ifndef ECHOSERVICE_H
#define ECHOSERVICE_H

#include <QtService/Service>
#include <QtNetwork/QTcpServer>

class EchoService : public QtService::Service
{
	Q_OBJECT

public:
	explicit EchoService(int &argc, char **argv);

protected:
	bool preStart() override;
	CommandResult onStart() override;
	CommandResult onStop(int &exitCode) override;
	CommandResult onReload() override;
	CommandResult onPause() override;
	CommandResult onResume() override;

private Q_SLOTS:
	void newConnection();

private:
	QTcpServer *_server = nullptr;

	static QByteArray host(QTcpSocket *socket);
};

#endif // ECHOSERVICE_H
