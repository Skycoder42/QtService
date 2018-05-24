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
	void start() override;
	void stop() override;
	void pause() override;
	void resume() override;
	void reload() override;

private Q_SLOTS:
	void newConnection();

private:
	QTcpServer *_server = nullptr;

	static QByteArray host(QTcpSocket *socket);
};

#endif // ECHOSERVICE_H
