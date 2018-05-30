#ifndef QTSERVICE_TERMINAL_P_H
#define QTSERVICE_TERMINAL_P_H

#include <QtCore/QDataStream>
#include <QtNetwork/QLocalSocket>

#include "qtservice_global.h"
#include "terminal.h"

namespace QtService {

// exported as generally terminals are created from their private component
class Q_SERVICE_EXPORT TerminalPrivate : public QObject
{
	Q_OBJECT
	friend class QtService::Terminal;

public:
	enum RequestType {
		CharRequest = 1,
		MultiCharRequest = 2,
		LineRequest = 3
	};
	Q_ENUM(RequestType)

	TerminalPrivate(QLocalSocket *socket, QObject *parent = nullptr);

Q_SIGNALS:
	void statusLoadComplete(TerminalPrivate *terminal, bool successful);

private Q_SLOTS:
	void disconnected();
	void error();
	void readyRead();

private:
	QLocalSocket *socket;

	Service::TerminalMode terminalMode = Service::ReadWriteActive;
	QStringList command;
	bool autoDelete = true;

	bool isLoading = true;
	QDataStream commandStream;
};

}

#endif // QTSERVICE_TERMINAL_P_H
