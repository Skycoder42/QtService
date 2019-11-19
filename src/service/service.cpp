#include "service.h"
#include "service_p.h"
#include "serviceplugin.h"
#include "terminalclient_p.h"
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include "servicecontrol.h"

#include <QtCore/private/qfactoryloader_p.h>

namespace {

class ServiceFactory : public QFactoryLoader
{
public:
	template <typename... TArgs>
	ServiceFactory(TArgs&&... args) :
		  QFactoryLoader{std::forward<TArgs>(args)...}
	{}

	QtService::ServicePlugin *instance(const QString &key) const {
		const auto index = indexOf(key);
		if (index != -1) {
			const auto factoryObject = QFactoryLoader::instance(index);
			if (const auto factory = qobject_cast<QtService::ServicePlugin*>(factoryObject); factory)
				return factory;
		}
		return nullptr;
	}

	inline QString currentServiceId(const QString &backend) const {
		const auto inst = instance(backend);
		return inst ? inst->currentServiceId(backend) : QString{};
	}

	inline QString findServiceId(const QString &backend, const QString &serviceName, const QString &domain) const {
		const auto inst = instance(backend);
		return inst ? inst->findServiceId(backend, serviceName, domain) : QString{};
	}

	inline QtService::ServiceBackend *createServiceBackend(const QString &backend, QtService::Service *service) {
		const auto inst = instance(backend);
		return inst ? inst->createServiceBackend(backend, service) : nullptr;
	}

	inline QtService::ServiceControl *createServiceControl(const QString &backend, QString &&serviceId, QObject *parent) {
		const auto inst = instance(backend);
		return inst ? inst->createServiceControl(backend, std::move(serviceId), parent) : nullptr;
	}
};

}

Q_GLOBAL_STATIC_WITH_ARGS(ServiceFactory, loader,
						  (QtService_ServicePlugin_Iid, QLatin1String("/servicebackends")))

using namespace QtService;

Q_LOGGING_CATEGORY(QtService::logSvc, "qt.service.service");

Service::Service(int &argc, char **argv, int flags) :
	QObject{},
	d{new ServicePrivate{this, argc, argv, flags}}
{
	Q_ASSERT_X(!ServicePrivate::instance, Q_FUNC_INFO, "There can always be only 1 QtService::Service instance at a time");
	ServicePrivate::instance = this;
}

int Service::exec()
{
	d->backendProvider = QStringLiteral("standard");
	auto backendFound = false;
	auto asTerminal = false;
	for (auto i = 1; i < d->argc; ++i) {
		QByteArray arg {d->argv[i]};
		if (!backendFound && arg == "--backend") {
			if (i+1 < d->argc) {
				backendFound = true;
				d->backendProvider = QString::fromUtf8(d->argv[i+1]);
			} else {
				qCCritical(logSvc) << "You must specify the backend name after the \"--backend\" parameter";
				return EXIT_FAILURE;
			}
		} else if(!asTerminal && arg == "--terminal")
			asTerminal = true;
	}
	qCDebug(logSvc) << "Using backend" << d->backendProvider
					<< "with terminals" << (asTerminal ? "enabled" : "disabled");

	if(asTerminal) {
		if(d->terminalActive) {
			TerminalClient client{this};
			return client.exec(d->argc, d->argv, d->flags);
		} else {
			qCCritical(logSvc) << "Terminal mode has not been enabled! See QtService::Service::terminalActive";
			return EXIT_FAILURE;
		}
	} else {
		d->backend = loader->createServiceBackend(d->backendProvider, this);
		if (!d->backend) {
			qCCritical(logSvc) << "No backend found for the name" << d->backendProvider;
			return EXIT_FAILURE;
		}
		return d->backend->runService(d->argc, d->argv, d->flags);
	}
}

Service *Service::instance()
{
	return ServicePrivate::instance;
}

QList<int> Service::getSockets(const QByteArray &socketName)
{
	return d->backend->getActivatedSockets(socketName);
}

int Service::getSocket()
{
	const auto sockets = getSockets({});
	if (sockets.isEmpty())
		return -1;
	else if (sockets.size() > 1)
		qCWarning(logSvc) << "Found" << sockets.size() << "default sockets - returning the first one only";
	return sockets.first();
}

QString Service::backend() const
{
	return d->backendProvider;
}

QDir Service::runtimeDir() const
{
	return ServicePrivate::runtimeDir();
}

bool Service::isTerminalActive() const
{
	return d->terminalActive;
}

Service::TerminalMode Service::terminalMode() const
{
	return d->terminalMode;
}

bool Service::isGlobalTerminal() const
{
	return d->terminalGlobal;
}

bool Service::startWithTerminal() const
{
	return d->startWithTerminal;
}

void Service::quit()
{
	d->backend->quitService();
}
void Service::reload()
{
	d->backend->reloadService();
}

void Service::setTerminalActive(bool terminalActive)
{
	if (d->terminalActive == terminalActive)
		return;

	d->terminalActive = terminalActive;
	if(terminalActive)
		d->startTerminals();
	else
		d->stopTerminals();
	emit terminalActiveChanged(d->terminalActive, {});
}

void Service::setTerminalMode(Service::TerminalMode terminalMode)
{
	if (d->terminalMode == terminalMode)
		return;

	d->terminalMode = terminalMode;
	emit terminalModeChanged(d->terminalMode, {});
}

void Service::setGlobalTerminal(bool globalTerminal)
{
	if (d->terminalGlobal == globalTerminal)
		return;

	if(d->termServer && d->termServer->isRunning())
		qCWarning(logSvc) << "Chaning the globalTerminal property will not have any effect until you disable and reenable the terminal server";
	d->terminalGlobal = globalTerminal;
	emit globalTerminalChanged(d->terminalGlobal, {});
}

void Service::setStartWithTerminal(bool startWithTerminal)
{
	if (d->startWithTerminal == startWithTerminal)
		return;

	d->startWithTerminal = startWithTerminal;
	emit startWithTerminalChanged(d->startWithTerminal, {});
}

void Service::terminalConnected(Terminal *terminal)
{
	qCWarning(logSvc) << "Terminal connected but was not handled - disconnecting it again";
	terminal->disconnectTerminal();
}

bool Service::preStart()
{
	return true;
}

Service::CommandResult Service::onStop(int &exitCode)
{
	Q_UNUSED(exitCode)
	return CommandResult::Completed;
}

Service::CommandResult Service::onReload()
{
	return CommandResult::Completed;
}

Service::CommandResult Service::onPause()
{
	return CommandResult::Completed;
}

Service::CommandResult Service::onResume()
{
	return CommandResult::Completed;
}

QVariant Service::onCallback(const QByteArray &kind, const QVariantList &args)
{
	if (d->callbacks.contains(kind)) {
		qCDebug(logSvc) << "Found and calling dynamic callback named" << kind;
		return d->callbacks[kind](args);
	} else {
		qCWarning(logSvc) << "Unhandeled callback of kind" << kind;
		return {};
	}
}

bool Service::verifyCommand(const QStringList &arguments)
{
	Q_UNUSED(arguments)
	return true;
}

void Service::addCallback(const QByteArray &kind, const std::function<QVariant (QVariantList)> &fn)
{
	Q_ASSERT_X(fn, Q_FUNC_INFO, "fn must be a valid function");
	d->callbacks.insert(kind, fn);
	qCDebug(logSvc) << "Registered dynamic callback for name" << kind;
}

Service::~Service() = default;

// ------------- Private Implementation -------------

QPointer<Service> ServicePrivate::instance{nullptr};

ServicePrivate::ServicePrivate(Service *q_ptr, int &argc, char **argv, int flags) :
	argc{argc},
	argv{argv},
	flags{flags},
	q{q_ptr}
{}

QStringList ServicePrivate::listBackends()
{
	return loader->keyMap().values();
}

QString ServicePrivate::idFromName(const QString &provider, const QString &serviceName, const QString &domain)
{
	return loader->findServiceId(provider, serviceName, domain);
}

ServiceControl *ServicePrivate::createControl(const QString &provider, QString &&serviceId, QObject *parent)
{
	return loader->createServiceControl(provider, std::move(serviceId), parent);
}

ServiceControl *ServicePrivate::createLocalControl(const QString &provider, QObject *parent)
{
	return ServiceControl::create(provider,
								  loader->currentServiceId(provider),
								  QCoreApplication::applicationName(), // make shure we get the same serviceName, even if the serviceId suggests otherwise
								  parent);
}

QDir ServicePrivate::runtimeDir(const QString &serviceName)
{
	QString runRoot;
#ifdef Q_OS_UNIX
	if (::geteuid() == 0)
		runRoot = QStringLiteral("/run");
	else
#endif
		runRoot = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
	if (runRoot.isEmpty())
		runRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	if (runRoot.isEmpty())
		return QDir::current();

	QDir runDir {runRoot};
	if (!runDir.exists(serviceName)) {
		if(!runDir.mkpath(serviceName))
			return QDir::current();
		if(!QFile::setPermissions(runDir.absoluteFilePath(serviceName),
								   QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner))
			qCWarning(logSvc) << "Failed to set permissions on runtime dir";
	}
	if (runDir.cd(serviceName))
		return runDir;
	else
		return QDir::current();
}

void ServicePrivate::startTerminals()
{
	if (!terminalActive || !isRunning)
		return;

	if (!termServer) {
		termServer = new TerminalServer{q};
		QObject::connect(termServer, &TerminalServer::terminalConnected,
						 q, &Service::terminalConnected);
	}
	terminalActive = termServer->start(terminalGlobal);
}

void ServicePrivate::stopTerminals()
{
	if (termServer)
		termServer->stop();
}
