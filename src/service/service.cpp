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
		return d->backend->runService(this, d->argc, d->argv, d->flags);
	} catch(QPluginLoadException &e) {
		qCCritical(logQtService) << "Failed to load backend" << provider << "with error:" << e.what();
		return EXIT_FAILURE;
	}
}

Service *Service::instance()
{
	return ServicePrivate::instance;
}

int Service::getSocket()
{
	const auto sockets = getAllSockets();
	if(sockets.isEmpty())
		return -1;
	else if(sockets.size() > 0)
		qCWarning(logQtService) << "Found" << sockets.size() << "activated sockets instead of just 1";
	return sockets.first();
}

QList<int> Service::getAllSockets()
{
	return getAllSocketsNamed().keys();
}

QHash<int, QByteArray> Service::getAllSocketsNamed()
{
	return d->backend->getActivatedSockets();
}

QString Service::backend() const
{
	return d->backendProvider;
}

void Service::quit()
{
	d->backend->quitService();
}

void Service::stopCompleted(int exitCode)
{
	emit stopped(exitCode, {});
}

bool Service::preStart()
{
	return true;
}

void Service::stop()
{
	stopCompleted();
}

void Service::pause() {}

void Service::resume() {}

void Service::reload() {}

void Service::processCommand(int code)
{
	switch(code) {
	case PauseCode:
		pause();
		break;
	case ResumeCode:
		resume();
		break;
	case ReloadCode:
		reload();
		break;
	default:
		qCWarning(logQtService) << "Unhandled command received:" << code;
		break;
	}
}

#ifdef Q_OS_ANDROID
QAndroidBinder *Service::onBind(const QAndroidIntent &intent)
{
	Q_UNUSED(intent)
	return nullptr;
}
#endif

Service::~Service() = default;

// ------------- Private Implementation -------------

QPointer<Service> ServicePrivate::instance{nullptr};

ServicePrivate::ServicePrivate(int &argc, char **argv, int flags) :
	argc{argc},
	argv{argv},
	flags{flags}
{}
