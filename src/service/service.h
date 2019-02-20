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

//! The primary namespace of the QtService library
namespace QtService {

class Terminal;
class TerminalClient;
class ServiceBackend;
class ServicePrivate;
//! The main interface to implement to create a service
class Q_SERVICE_EXPORT Service : public QObject
{
	Q_OBJECT

	//! The backend this service is currently run with
	Q_PROPERTY(QString backend READ backend CONSTANT)
	//! The runtime directory this service should place runtime files in
	Q_PROPERTY(QDir runtimeDir READ runtimeDir CONSTANT)

	//! Specifies whether the service accepts terminals
	Q_PROPERTY(bool terminalActive READ isTerminalActive WRITE setTerminalActive NOTIFY terminalActiveChanged)
	//! Specifies the mode new terminals are started as
	Q_PROPERTY(TerminalMode terminalMode READ terminalMode WRITE setTerminalMode NOTIFY terminalModeChanged)
	//! Specifies whether terminals from all users can connect, or only of the same user as the service
	Q_PROPERTY(bool globalTerminal READ isGlobalTerminal WRITE setGlobalTerminal NOTIFY globalTerminalChanged)
	//! Specifies whether terminals should try to start the service if it is not running
	Q_PROPERTY(bool startWithTerminal READ startWithTerminal WRITE setStartWithTerminal NOTIFY startWithTerminalChanged)

public:
	//! Indicates whether a service command has finished or needs to run asynchronously
	enum CommandResult {
		OperationCompleted, //!< The command was successfully completed synchronously
		OperationPending, //!< The command is beeing proccessed asynchronously and the service will emit the corresponding signal once it's done
		OperationFailed, //!< The command failed synchronously. The system may exit the service afterwards

		OperationExit, //!< The command executed successfully, but the service should still exit. Only usable from onStart()

		//MAJOR compat remove
		Synchronous Q_DECL_ENUMERATOR_DEPRECATED = OperationCompleted, //!< @deprecated Deprecated. Use OperationCompleted instead
		Asynchronous Q_DECL_ENUMERATOR_DEPRECATED = OperationPending //!< @deprecated Deprecated. Use OperationPending instead
	};
	//! @deprecated Deprecated. Use CommandResult instead
	using CommandMode Q_DECL_ENUMERATOR_DEPRECATED = CommandResult;//MAJOR compat remove
	Q_ENUM(CommandResult)

	//! The modes a terminal can be in
	enum TerminalMode {
		ReadOnly, //!< The terminal can only receive data from the service. Useful for machine-to-machine communication
		WriteOnly, //!< The terminal can only send data to the service. Useful for machine-to-machine communication
		ReadWritePassive, //!< The terminal can read and write as much as it wants. Useful for machine-to-machine communication
		ReadWriteActive //!< The terminal can read and write, but writing is limited to what the service requests. Recommended for humans using the terminal
	};
	Q_ENUM(TerminalMode)

	//! Constructs a new service from the main arguments
	explicit Service(int &argc, char **argv, int = QCoreApplication::ApplicationFlags);
	~Service() override;

	//! Starts the service execution
	int exec();

	//! Returns a reference to the currently running service
	static Service *instance();

	//! Returns all activated sockets found for the given name
	Q_INVOKABLE QList<int> getSockets(const QByteArray &socketName);
	//! Returns the default activated socket, if one exists
	Q_INVOKABLE int getSocket();

	//! @readAcFn{Service::backend}
	QString backend() const;
	//! @readAcFn{Service::runtimeDir}
	QDir runtimeDir() const;
	//! @readAcFn{Service::terminalActive}
	bool isTerminalActive() const;
	//! @readAcFn{Service::terminalMode}
	TerminalMode terminalMode() const;
	//! @readAcFn{Service::globalTerminal}
	bool isGlobalTerminal() const;
	//! @readAcFn{Service::startWithTerminal}
	bool startWithTerminal() const;

public Q_SLOTS:
	//! Perform a graceful service stop
	void quit();
	//! Perform a reload command
	void reload();

	//! @writeAcFn{Service::terminalActive}
	void setTerminalActive(bool terminalActive);
	//! @writeAcFn{Service::terminalMode}
	void setTerminalMode(TerminalMode terminalMode);
	//! @writeAcFn{Service::globalTerminal}
	void setGlobalTerminal(bool globalTerminal);
	//! @writeAcFn{Service::startWithTerminal}
	void setStartWithTerminal(bool startWithTerminal);

Q_SIGNALS:
	//! Must be emitted when starting was completed if onStart returned OperationPending
	void started(bool success); //TODO doc should be direct connected
	//! @deprecated Use started(bool) instead
	Q_DECL_DEPRECATED void started();
	//! Must be emitted when stopping was completed if onStop returned OperationPending
	void stopped(int exitCode = EXIT_SUCCESS);
	//! Must be emitted when reloading was completed if onReload returned OperationPending
	void reloaded(bool success);
	//! @deprecated Use reloaded(bool) instead
	Q_DECL_DEPRECATED void reloaded();
	//! Must be emitted when pausing was completed if onPause returned OperationPending
	void paused(bool success);
	//! @deprecated Use paused(bool) instead
	Q_DECL_DEPRECATED void paused();
	//! Must be emitted when resuming was completed if onResume returned OperationPending
	void resumed(bool success);
	//! @deprecated Use resumed(bool) instead
	Q_DECL_DEPRECATED void resumed();

	//! @notifyAcFn{Service::terminalActive}
	void terminalActiveChanged(bool terminalActive, QPrivateSignal);
	//! @notifyAcFn{Service::terminalMode}
	void terminalModeChanged(TerminalMode terminalMode, QPrivateSignal);
	//! @notifyAcFn{Service::globalTerminal}
	void globalTerminalChanged(bool globalTerminal, QPrivateSignal);
	//! @notifyAcFn{Service::startWithTerminal}
	void startWithTerminalChanged(bool startWithTerminal, QPrivateSignal);

protected Q_SLOTS:
	//! Is called by the backend for every newly connected terminal
	virtual void terminalConnected(Terminal *terminal);

protected:
	//! Fallback method for otherwise impossible early setup (try to not use it)
	virtual bool preStart();

	//! Is called by the backend to start the service
	virtual CommandResult onStart() = 0;
	//! Is called by the backend to stop the service
	virtual CommandResult onStop(int &exitCode);
	//! Is called by the backend to reload the service
	virtual CommandResult onReload();
	//! Is called by the backend to pause the service
	virtual CommandResult onPause();
	//! Is called by the backend to resume the service
	virtual CommandResult onResume();

	//! Is called by the backend if a platform specific callback was triggered
	virtual QVariant onCallback(const QByteArray &kind, const QVariantList &args);

	//! Is called from the terminal process to perform early CLI validation
	Q_DECL_DEPRECATED virtual bool verifyCommand(const QStringList &arguments);
	virtual bool verifyCommand2(const QStringList &arguments);

	//! Adds a callback to be called by onCallback for the given kind
	void addCallback(const QByteArray &kind, const std::function<QVariant(QVariantList)> &fn);
	//! @copybrief Service::addCallback(const QByteArray &, const std::function<QVariant(QVariantList)> &)
	template <typename TFunction>
	void addCallback(const QByteArray &kind, const TFunction &fn);
	//! @copybrief Service::addCallback(const QByteArray &, const std::function<QVariant(QVariantList)> &)
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

//! A define for the service instance for easy use, see QtService::Service::instance
#define qService QtService::Service::instance()

//! @file service.h The Service header
#endif // QTSERVICE_SERVICE_H
