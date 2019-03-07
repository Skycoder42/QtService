#include "launchdservicecontrol.h"
#include "launchdserviceplugin.h"
#include <QtCore/QStandardPaths>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>

#include <unistd.h>
#include <sys/types.h>

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
	return SupportFlag::StartStop |
			SupportFlag::CustomCommands |
			SupportFlag::SetEnabled;
}

bool LaunchdServiceControl::serviceExists() const
{
	return runLaunchctl("list") == EXIT_SUCCESS;
}

bool LaunchdServiceControl::isEnabled() const
{
	const auto target = QStringLiteral("user/%1/")
						.arg(::geteuid());
	QByteArray outData;
	if (runLaunchctl("print-disabled", {target}, false, &outData) == EXIT_SUCCESS) {
		const QRegularExpression lineRegex{
			QStringLiteral(R"__(\"%1\"\s*=>\s*true)__").arg(serviceId()),
			QRegularExpression::DontCaptureOption
		};
		// if not explicitly disabled, assume enabled
		return !lineRegex.match(QString::fromUtf8(outData)).hasMatch();
	} else
		return true; // assume enabled by default
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

bool LaunchdServiceControl::setEnabled(bool enabled)
{
	if(enabled == isEnabled())
		return true;

	const auto target = QStringLiteral("user/%1/%2")
						.arg(::geteuid())
						.arg(serviceId());
	return runLaunchctl(enabled ? "enable" : "disable", {target}, false) == EXIT_SUCCESS;
}

QString LaunchdServiceControl::serviceName() const
{
	return serviceId().split(QLatin1Char('.')).last();
}

int LaunchdServiceControl::runLaunchctl(const QByteArray &command, const QStringList &extraArgs, bool withServiceId, QByteArray *outData) const
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
	if(withServiceId)
		args.append(serviceId());
	process.setArguments(args);

	process.setStandardInputFile(QProcess::nullDevice());
	if(!outData)
		process.setStandardOutputFile(QProcess::nullDevice());
	process.setProcessChannelMode(QProcess::ForwardedErrorChannel);

	process.start(QProcess::ReadOnly);
	if(process.waitForFinished(2500)) {  // non-blocking calls should finish within two seconds
		if(outData)
			*outData = process.readAllStandardOutput();
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
