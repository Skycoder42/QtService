#include "launchdservicecontrol.h"
#include "launchdserviceplugin.h"
#include <QtCore/QStandardPaths>
#include <QtCore/QProcess>
using namespace QtService;

LaunchdServiceControl::LaunchdServiceControl(QString &&serviceId, QObject *parent) :
	ServiceControl{std::move(serviceId), parent}
{}

QString LaunchdServiceControl::backend() const
{
	return QStringLiteral("launchd");
}

ServiceControl::SupportFlags LaunchdServiceControl::supportFlags() const
{
	return SupportsStartStop | SupportsCustomCommands | SupportsNonBlocking;
}

bool LaunchdServiceControl::serviceExists() const
{
	return runLaunchctl("list") == EXIT_SUCCESS;
}

QVariant LaunchdServiceControl::callGenericCommand(const QByteArray &kind, const QVariantList &args)
{
	QStringList sArgs;
	sArgs.reserve(args.size());
	for(const auto &arg : args)
		sArgs.append(arg.toString());
	return runLaunchctl(kind, sArgs);
}

bool LaunchdServiceControl::start()
{
	return runLaunchctl("start") == EXIT_SUCCESS;
}

bool LaunchdServiceControl::stop()
{
	return runLaunchctl("stop") == EXIT_SUCCESS;
}

QString LaunchdServiceControl::serviceName() const
{
	return serviceId().split(QLatin1Char('.')).last();
}

int LaunchdServiceControl::runLaunchctl(const QByteArray &command, const QStringList &extraArgs) const
{
	const auto launchctl = QStandardPaths::findExecutable(QStringLiteral("launchctl"));
	if(launchctl.isEmpty()) {
		setError(tr("Failed to find launchctl executable"));
		return -1;
	}

	QProcess process;
	process.setProgram(launchctl);

	QStringList args;
	args.reserve(extraArgs.size() + 2);
	args.append(QString::fromUtf8(command));
	args.append(extraArgs);
	args.append(serviceId());
	process.setArguments(args);

	process.setStandardInputFile(QProcess::nullDevice());
	process.setStandardOutputFile(QProcess::nullDevice());
	process.setProcessChannelMode(QProcess::ForwardedErrorChannel);

	process.start(QProcess::ReadOnly);
	if(process.waitForFinished(isBlocking() ? -1 : 2500)) {//non-blocking calls should finish within two seconds
		if(process.exitStatus() == QProcess::NormalExit)
			return process.exitCode();
		else {
			setError(tr("launchctl crashed with error: %1").arg(process.errorString()));
			return 128 + process.error();
		}
	} else {
		setError(tr("launchctl did not exit in time"));
		return -1;
	}
}
