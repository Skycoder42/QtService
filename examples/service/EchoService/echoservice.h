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
	CommandMode onStart() override;
	CommandMode onStop(int &exitCode) override;
	CommandMode onReload() override;
	void onPause() override;
	void onResume() override;

private Q_SLOTS:
	void newConnection();

private:
	QTcpServer *_server = nullptr;

	static QByteArray host(QTcpSocket *socket);
};

#endif // ECHOSERVICE_H
