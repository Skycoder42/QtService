#include "systemdservicecontrol.h"
#include <unistd.h>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QRegularExpression>
#include <QtService/private/logging_p.h>
using namespace QtService;

SystemdServiceControl::SystemdServiceControl(QString &&serviceId, QObject *parent) :
	ServiceControl{std::move(serviceId), parent}
{}

ServiceControl::SupportFlags SystemdServiceControl::supportFlags() const
{
	return SupportsStartStop |
			SupportsReload |
			SupportsEnableDisable |
			SupportsStatus |
			SupportsCustomCommands;
}

ServiceControl::ServiceStatus SystemdServiceControl::status() const
{
	return ServiceStatusUnknown;
}

bool SystemdServiceControl::isEnabled() const
{
	return false;
}

QString SystemdServiceControl::backend() const
{
	return QStringLiteral("systemd");
}

QVariant SystemdServiceControl::callGenericCommand(const QByteArray &kind, const QVariantList &args)
{
	QStringList sArgs;
	sArgs.reserve(args.size());
	for(const auto &arg : args)
		sArgs.append(arg.toString());
	return runSystemctl(kind, sArgs);
}

bool SystemdServiceControl::start()
{
	return runSystemctl("start") == EXIT_SUCCESS;
}

bool SystemdServiceControl::stop()
{
	return runSystemctl("stop") == EXIT_SUCCESS;
}

bool SystemdServiceControl::reload()
{
	return runSystemctl("reload") == EXIT_SUCCESS;
}

bool SystemdServiceControl::enable()
{
	return runSystemctl("enable") == EXIT_SUCCESS;
}

bool SystemdServiceControl::disable()
{
	return runSystemctl("disable") == EXIT_SUCCESS;
}

QString SystemdServiceControl::serviceName() const
{
	const static QRegularExpression regex(QStringLiteral(R"__((.+)\.(?:service|socket|device|mount|automount|swap|target|path|timer|slice|scope))__"));
	const auto svcId = serviceId();
	auto match = regex.match(svcId);
	if(match.hasMatch())
		return match.captured(1);
	else
		return svcId;
}

int SystemdServiceControl::runSystemctl(const QByteArray &command, const QStringList &extraArgs, QByteArray *outData) const
{
	const auto systemctl = QStandardPaths::findExecutable(QStringLiteral("systemctl"));
	if(systemctl.isEmpty()) {
		qCWarning(logQtService) << "Failed to find systemctl executable";
		return EXIT_FAILURE;
	}

	QProcess process;
	process.setProgram(systemctl);

	QStringList args;
	args.reserve(extraArgs.size() + 4);
	if(::geteuid() == 0)
		args.append(QStringLiteral("--system"));
	else
		args.append(QStringLiteral("--user"));
	args.append(QString::fromUtf8(command));
	args.append(serviceId());
	if(!isBlocking())
		args.append(QStringLiteral("--no-block"));
	args.append(extraArgs);
	process.setArguments(extraArgs);

	process.setStandardInputFile(QProcess::nullDevice());
	if(!outData)
		process.setStandardOutputFile(QProcess::nullDevice());
	process.setProcessChannelMode(QProcess::ForwardedErrorChannel);

	process.start(QProcess::ReadOnly);
	if(process.waitForFinished(isBlocking() ? -1 : 2500)) {//non-blocking calls should finish within two seconds
		if(process.exitStatus() == QProcess::NormalExit) {
			auto code = process.exitCode();
			if(code != EXIT_SUCCESS)
				qCWarning(logQtService) << "systemctl failed with exit code:" << code;
			return code;
		} else {
			qCWarning(logQtService).noquote() << "systemctl crashed with error:" << process.errorString();
			return 128 + process.error();
		}
	} else {
		qCWarning(logQtService).noquote() << "systemctl did not exit in time";
		return EXIT_FAILURE;
	}
}
