#ifndef QTSERVICE_SERVICE_H
#define QTSERVICE_SERVICE_H

#include <functional>

#include <QtCore/qobject.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qvector.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>

#include "QtService/qtservice_global.h"
#include "QtService/qtservice_helpertypes.h"

namespace QtService {

class Terminal;
class TerminalClient;
class ServiceBackend;
class ServicePrivate;
class Q_SERVICE_EXPORT Service : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString backend READ backend CONSTANT)
	Q_PROPERTY(QDir runtimeDir READ runtimeDir CONSTANT)

	Q_PROPERTY(bool terminalActive READ isTerminalActive WRITE setTerminalActive NOTIFY terminalActiveChanged)
	Q_PROPERTY(TerminalMode terminalMode READ terminalMode WRITE setTerminalMode NOTIFY terminalModeChanged)
	Q_PROPERTY(bool globalTerminal READ globalTerminal WRITE setGlobalTerminal NOTIFY globalTerminalChanged)
	Q_PROPERTY(bool startWithTerminal READ startWithTerminal WRITE setStartWithTerminal NOTIFY startWithTerminalChanged)

public:
	enum CommandMode {
		Synchronous,
		Asynchronous
	};
	Q_ENUM(CommandMode)

	enum TerminalMode {
		ReadOnly,
		WriteOnly,
		ReadWritePassive,
		ReadWriteActive
	};
	Q_ENUM(TerminalMode)

	explicit Service(int &argc, char **argv, int = QCoreApplication::ApplicationFlags);
	~Service() override;

	int exec();

	static Service *instance();

	QList<int> getSockets(const QByteArray &socketName);
	int getSocket();

	QString backend() const;
	QDir runtimeDir() const;
	bool isTerminalActive() const;
	TerminalMode terminalMode() const;
	bool globalTerminal() const;
	bool startWithTerminal() const;

public Q_SLOTS:
	void quit();
	void reload();

	void setTerminalActive(bool terminalActive);
	void setTerminalMode(TerminalMode terminalMode);
	void setGlobalTerminal(bool globalTerminal);
	void setStartWithTerminal(bool startWithTerminal);

Q_SIGNALS:
	void started();
	void stopped(int exitCode = EXIT_SUCCESS);
	void reloaded();
	void paused();
	void resumed();

	void terminalActiveChanged(bool terminalActive, QPrivateSignal);
	void terminalModeChanged(TerminalMode terminalMode, QPrivateSignal);
	void globalTerminalChanged(bool globalTerminal, QPrivateSignal);
	void startWithTerminalChanged(bool startWithTerminal, QPrivateSignal);

protected Q_SLOTS:
	virtual void terminalConnected(Terminal *terminal);

protected:
	virtual bool preStart();

	virtual CommandMode onStart() = 0;
	virtual CommandMode onStop(int &exitCode);
	virtual CommandMode onReload();
	virtual CommandMode onPause();
	virtual CommandMode onResume();

	virtual QVariant onCallback(const QByteArray &kind, const QVariantList &args);

	virtual bool verifyCommand(const QStringList &arguments);

	void addCallback(const QByteArray &kind, const std::function<QVariant(QVariantList)> &fn);
	template <typename TFunction>
	void addCallback(const QByteArray &kind, const TFunction &fn);
	template <typename TClass, typename TReturn, typename... TArgs>
	void addCallback(const QByteArray &kind, TReturn(TClass::*fn)(TArgs...), std::enable_if_t<std::is_base_of<QtService::Service, TClass>::value, void*> = nullptr);

private:
	friend class QtService::ServiceBackend;
	friend class QtService::ServicePrivate;
	friend class QtService::TerminalClient;

	QScopedPointer<ServicePrivate> d;
};

template<typename TFunction>
void Service::addCallback(const QByteArray &kind, const TFunction &fn)
{
	addCallback(kind, __helpertypes::pack_function(fn));
}

template<typename TClass, typename TReturn, typename... TArgs>
void Service::addCallback(const QByteArray &kind, TReturn (TClass::*fn)(TArgs...), std::enable_if_t<std::is_base_of<QtService::Service, TClass>::value, void*>)
{
	auto self = static_cast<TClass*>(this);
	addCallback(kind, [self, fn](TArgs... args) -> TReturn {
		return (self->*fn)(args...);
	});
}

}

#endif // QTSERVICE_SERVICE_H
