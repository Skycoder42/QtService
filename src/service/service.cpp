#include "service.h"
#include "service_p.h"
#include "serviceplugin.h"
#include "logging_p.h"
#include <QFileInfo>
#include <QDir>

#include <qpluginfactory.h>
using namespace QtService;

Q_LOGGING_CATEGORY(logQtService, "qtservice"); //TODO, QtInfoMsg);
Q_GLOBAL_PLUGIN_OBJECT_FACTORY(ServicePlugin, ServiceBackend, "servicebackends", factory)

Service::Service(int &argc, char **argv, int flags) :
	QObject{},
	d{new ServicePrivate{argc, argv, flags}}
{
	Q_ASSERT_X(!ServicePrivate::instance, Q_FUNC_INFO, "There can always be only 1 QtService::Service instance at a time");
	ServicePrivate::instance = this;
}

int Service::exec()
{
	QByteArray provider {"standard"};
	for(auto i = 1; i < d->argc; i++) {
		QByteArray arg {d->argv[i]};
		if(arg == "--backend") {
			if(i+1 < d->argc) {
				provider = d->argv[i+1];
				break;
			} else {
				qCCritical(logQtService) << "You must specify the backend name after the \"--backend\" parameter";
				return EXIT_FAILURE;
			}
		}
	}

	try {
		d->backendProvider = QString::fromUtf8(provider);
		d->backend = factory->createInstance(d->backendProvider, this);
		if(!d->backend) {
			qCCritical(logQtService) << "No backend found for the name" << provider;
			return EXIT_FAILURE;
		}
		return d->backend->runService(d->argc, d->argv, d->flags);
	} catch(QPluginLoadException &e) {
		qCCritical(logQtService) << "Failed to load backend" << provider << "with error:" << e.what();
		return EXIT_FAILURE;
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
	if(sockets.isEmpty())
		return -1;
	else if(sockets.size() > 1)
		qCWarning(logQtService) << "Found" << sockets.size() << "default sockets - returning the first one only";
	return sockets.first();
}

QString Service::backend() const
{
	return d->backendProvider;
}

void Service::quit()
{
	d->backend->quitService();
}
void Service::reload()
{
	d->backend->reloadService();
}

bool Service::preStart()
{
	return true;
}

Service::CommandMode Service::onStop(int &exitCode)
{
	Q_UNUSED(exitCode);
	return Synchronous;
}

Service::CommandMode Service::onReload()
{
	return Synchronous;
}

Service::CommandMode Service::onPause()
{
	return Synchronous;
}

Service::CommandMode Service::onResume()
{
	return Synchronous;
}

QVariant Service::onCallback(const QByteArray &kind, const QVariantList &args)
{
	if(d->callbacks.contains(kind))
		return d->callbacks[kind](args);
	else {
		qCWarning(logQtService) << "Unhandeled callback of kind" << kind;
		return {};
	}
}

void Service::addCallback(const QByteArray &kind, const std::function<QVariant (QVariantList)> &fn)
{
	Q_ASSERT_X(fn, Q_FUNC_INFO, "fn must be a valid function");
	d->callbacks.insert(kind, fn);
}

Service::~Service() = default;

// ------------- Private Implementation -------------

QPointer<Service> ServicePrivate::instance{nullptr};

ServicePrivate::ServicePrivate(int &argc, char **argv, int flags) :
	argc{argc},
	argv{argv},
	flags{flags}
{}
