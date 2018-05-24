#include "service.h"
#include "service_p.h"
#include "serviceplugin.h"
#include <QDebug>

#include <qpluginfactory.h>
using namespace QtService;

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
	QByteArray provider {"dummy"};
	for(auto i = 1; i < d->argc; i++) {
		QByteArray arg {d->argv[i]};
		if(arg == "--backend") {
			if(i+1 < d->argc) {
				provider = d->argv[i+1];
				break;
			} else {
				qCritical() << "You must specify the backend name after the \"--backend\" parameter";
				return EXIT_FAILURE;
			}
		}
	}

	try {
		auto backend = factory->createInstance(QString::fromUtf8(provider), this);
		if(!backend) {
			qCritical() << "No backend found for the name" << provider;
			return EXIT_FAILURE;
		}
		return backend->runService(this, d->argc, d->argv, d->flags);
	} catch(QPluginLoadException &e) {
		qCritical() << "Failed to load backend" << provider << "with error:" << e.what();
		return EXIT_FAILURE;
	}
}

Service *Service::instance()
{
	return ServicePrivate::instance;
}

int Service::getSocket()
{
	Q_UNIMPLEMENTED();
	return 0;
}

QVector<int> Service::getAllSockets()
{
	Q_UNIMPLEMENTED();
	return {};
}

QHash<int, QByteArray> Service::getAllSocketsNamed()
{
	Q_UNIMPLEMENTED();
	return {};
}

bool Service::preStart()
{
	return true;
}

void Service::stop() {}

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
		// do nothing
		break;
	}
}

void Service::pause() {}

void Service::resume() {}

void Service::reload() {}

#ifdef Q_OS_ANDROID
QAndroidBinder *Service::onBind(const QAndroidIntent &intent)
{
	Q_UNIMPLEMENTED();
	return {};
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
