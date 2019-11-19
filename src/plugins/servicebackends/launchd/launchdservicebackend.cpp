#include "launchdservicebackend.h"
#include "launchdserviceplugin.h"
#include <csignal>
#include <unistd.h>
#include <launch.h>
#include <syslog.h>
#include <QtCore/QScopedPointer>
using namespace QtService;

Q_LOGGING_CATEGORY(logBackend, "qt.service.plugin.launchd.backend")

LaunchdServiceBackend::LaunchdServiceBackend(Service *service) :
	ServiceBackend{service}
{}

int LaunchdServiceBackend::runService(int &argc, char **argv, int flags)
{
	qInstallMessageHandler(LaunchdServiceBackend::syslogMessageHandler);
	QCoreApplication app(argc, argv, flags);
	if (!preStartService())
		return EXIT_FAILURE;

	connect(service(), QOverload<bool>::of(&Service::started),
			this, &LaunchdServiceBackend::onStarted);
	connect(service(), QOverload<bool>::of(&Service::paused),
			this, &LaunchdServiceBackend::onPaused);

	for(const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT, SIGUSR1, SIGUSR2})
		registerForSignal(signal);

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, ServiceCommand::Start));
	return app.exec();
}

void LaunchdServiceBackend::quitService()
{
	connect(service(), &Service::stopped,
			qApp, &QCoreApplication::exit,
			Qt::UniqueConnection);
	processServiceCommand(ServiceCommand::Stop);
}

void LaunchdServiceBackend::reloadService()
{
	processServiceCommand(ServiceCommand::Reload);
}

QList<int> LaunchdServiceBackend::getActivatedSockets(const QByteArray &name)
{
	auto mName = name.isNull() ? QByteArrayLiteral("Listeners") : name;
	if (!_socketCache.contains(mName)) {
		int *fdsRaw = nullptr;
		size_t cnt = 0;
		int err = launch_activate_socket(mName.constData(), &fdsRaw, &cnt);
		QScopedPointer<int, QScopedPointerPodDeleter> fds{fdsRaw};
		if (err != 0)
			qCWarning(logBackend) << "Failed to get sockets with error:" << qUtf8Printable(qt_error_string(err));
		else {
			_socketCache.reserve(_socketCache.size() + static_cast<int>(cnt));
			for(size_t i = 0; i < cnt; i++)
				_socketCache.insert(mName, fds.data()[i]);
		}
	}

	return _socketCache.values(mName);
}

void LaunchdServiceBackend::signalTriggered(int signal)
{
	qCDebug(logBackend) << "Handling signal" << signal;
	switch(signal) {
	case SIGINT:
	case SIGTERM:
	case SIGQUIT:
		quitService();
		break;
	case SIGHUP:
		reloadService();
		break;
	case SIGTSTP:
		processServiceCommand(ServiceCommand::Pause);
		break;
	case SIGCONT:
		processServiceCommand(ServiceCommand::Resume);
		break;
	case SIGUSR1:
		processServiceCallback("SIGUSR1");
		break;
	case SIGUSR2:
		processServiceCallback("SIGUSR2");
		break;
	default:
		ServiceBackend::signalTriggered(signal);
		break;
	}
}

void LaunchdServiceBackend::onStarted(bool success)
{
	if (!success)
		qApp->exit(EXIT_FAILURE);
}

void LaunchdServiceBackend::onPaused(bool success)
{
	if (success)
		kill(getpid(), SIGSTOP);  // now actually stop
}

void LaunchdServiceBackend::syslogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
	auto formattedMessage = qFormatLogMessage(type, context, message);

	int priority;
	switch (type) {
	case QtDebugMsg:
		priority = LOG_DEBUG;
		break;
	case QtInfoMsg:
		priority = LOG_INFO;
		break;
	case QtWarningMsg:
		priority = LOG_WARNING;
		break;
	case QtCriticalMsg:
		priority = LOG_CRIT;
		break;
	case QtFatalMsg:
		priority = LOG_ALERT;
		break;
	default:
		Q_UNREACHABLE();
		break;
	}

	syslog(priority, "%s", qUtf8Printable(formattedMessage));
}
