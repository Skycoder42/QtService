#ifndef TESTSERVICE_H
#define TESTSERVICE_H

#include <QtService/Service>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtCore/QDataStream>

class TestService : public QtService::Service
{
	Q_OBJECT

public:
	explicit TestService(int &argc, char **argv);

protected:
	bool preStart() override;
	CommandMode onStart() override;
	CommandMode onStop(int &exitCode) override;
	CommandMode onReload() override;
	CommandMode onPause() override;
	CommandMode onResume() override;
	QVariant onCallback(const QByteArray &kind, const QVariantList &args) override;

private:
	QLocalServer *_server = nullptr;
	QLocalSocket *_socket = nullptr;
	QDataStream _stream;
};

#endif // TESTSERVICE_H
