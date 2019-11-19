#include "systemdservicecontrol.h"
#include "systemdserviceplugin.h"
#include <unistd.h>
#include <QtCore/QBuffer>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QRegularExpression>
using namespace QtService;

Q_LOGGING_CATEGORY(logControl, "qt.service.plugin.systemd.control")

SystemdServiceControl::SystemdServiceControl(QString &&serviceId, QObject *parent) :
	ServiceControl{std::move(serviceId), parent},
	_runAsUser{::geteuid() != 0}
{}

QString SystemdServiceControl::backend() const
{
	return QStringLiteral("systemd");
}

ServiceControl::SupportFlags SystemdServiceControl::supportFlags() const
{
	return SupportFlag::StartStop |
			SupportFlag::Reload |
			SupportFlag::Autostart |
			SupportFlag::Status |
			SupportFlag::CustomCommands |
			SupportFlag::SetBlocking;
}

bool SystemdServiceControl::serviceExists() const
{
	_svcInfo = SvcExists::No;
	auto svcName = serviceId().toUtf8();
	auto svcType = serviceId().mid(realServiceName().size() + 1);
	if (svcType.isEmpty()) {
		svcType = QStringLiteral("service");
		svcName += '.' + svcType.toUtf8();
	}
	qCDebug(logControl) << "Detected service type as:" << svcType;

	QByteArray data;
	if (runSystemctl("list-unit-files", {
						QStringLiteral("--all"),
						QStringLiteral("--full"),
						QStringLiteral("--no-pager"),
						QStringLiteral("--plain"),
						QStringLiteral("--all"),
						QStringLiteral("--no-legend"),
						QStringLiteral("--type=") + svcType
					}, &data, true) != EXIT_SUCCESS)
		return false;

	QBuffer buffer{&data};
	buffer.open(QIODevice::ReadOnly);
	while (!buffer.atEnd()) {
		// read the line and check if it is this service, and if yes "parse" the line and verify again
		auto line = buffer.readLine();
		if (!line.startsWith(svcName))
			continue;
		auto lineData = line.simplified().split(' ');
		if (lineData[0] != svcName)
			continue;
		_svcInfo = SvcExists::Yes;
		return true;
	}

	return false;
}

ServiceControl::Status SystemdServiceControl::status() const
{
	auto svcName = serviceId().toUtf8();
	auto svcType = serviceId().mid(realServiceName().size() + 1);
	if (svcType.isEmpty()) {
		svcType = QStringLiteral("service");
		svcName += '.' + svcType.toUtf8();
	}
	qCDebug(logControl) << "Detected service type as:" << svcType;

	QByteArray data;
	if (runSystemctl("list-units", {
						QStringLiteral("--all"),
						QStringLiteral("--full"),
						QStringLiteral("--no-pager"),
						QStringLiteral("--plain"),
						QStringLiteral("--all"),
						QStringLiteral("--no-legend"),
						QStringLiteral("--type=") + svcType
					}, &data, true) != EXIT_SUCCESS)
		return Status::Unknown;

	QBuffer buffer{&data};
	buffer.open(QIODevice::ReadOnly);
	while (!buffer.atEnd()) {
		// read the line and check if it is this service, and if yes "parse" the line and verify again
		auto line = buffer.readLine();
		if (!line.startsWith(svcName))
			continue;
		auto lineData = line.simplified().split(' ');
		if (lineData.size() < 3 || lineData[0] != svcName)
			continue;

		// found correct service! now read the status
		const auto &svcState = lineData[2];
		if (svcState == "active")
			return Status::Running;
		else if (svcState == "reloading")
			return Status::Reloading;
		else if (svcState == "inactive")
			return Status::Stopped;
		else if (svcState == "failed")
			return Status::Errored;
		else if (svcState == "activating")
			return Status::Starting;
		else if (svcState == "deactivating")
			return Status::Stopping;
		else {
			setError(tr("Unknown service state %1 for service %2")
					 .arg(QString::fromUtf8(svcState), QString::fromUtf8(svcName)));
			return Status::Unknown;
		}
	}

	if (_svcInfo == SvcExists::Unknown)
		serviceExists();
	if (_svcInfo == SvcExists::Yes)
		return Status::Stopped;
	else {
		setError(tr("Service %1 was not found as systemd service")
				 .arg(QString::fromUtf8(svcName)));
		return Status::Unknown;
	}
}

bool SystemdServiceControl::isAutostartEnabled() const
{
	return runSystemctl("is-enabled") == EXIT_SUCCESS;
}

ServiceControl::BlockMode SystemdServiceControl::blocking() const
{
	return _blocking ? BlockMode::Blocking : BlockMode::NonBlocking;
}

bool SystemdServiceControl::isRunAsUser() const
{
	return _runAsUser;
}

QVariant SystemdServiceControl::callGenericCommand(const QByteArray &kind, const QVariantList &args)
{
	QStringList sArgs;
	sArgs.reserve(args.size());
	for (const auto &arg : args)
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

bool SystemdServiceControl::restart()
{
	return runSystemctl("restart") == EXIT_SUCCESS;
}

bool SystemdServiceControl::reload()
{
	return runSystemctl("reload") == EXIT_SUCCESS;
}

bool SystemdServiceControl::enableAutostart()
{
	return runSystemctl("enable") == EXIT_SUCCESS;
}

bool SystemdServiceControl::disableAutostart()
{
	return runSystemctl("disable") == EXIT_SUCCESS;
}

bool SystemdServiceControl::setBlocking(bool blocking)
{
	if (_blocking == blocking)
		return true;

	_blocking = blocking;
	emit blockingChanged(this->blocking());
	return true;
}

void SystemdServiceControl::setRunAsUser(bool runAsUser)
{
	if (_runAsUser == runAsUser)
		return;

	_runAsUser = runAsUser;
	emit runAsUserChanged(_runAsUser);
}

void SystemdServiceControl::resetRunAsUser()
{
	setRunAsUser(::geteuid() != 0);
}

QString SystemdServiceControl::serviceName() const
{
	const static QRegularExpression regex(QStringLiteral(R"__((.+)\.(?:service|socket|device|mount|automount|swap|target|path|timer|slice|scope))__"));
	const auto svcId = serviceId();
	auto match = regex.match(svcId);
	if (match.hasMatch())
		return match.captured(1);
	else
		return svcId;
}

int SystemdServiceControl::runSystemctl(const QByteArray &command, const QStringList &extraArgs, QByteArray *outData, bool noPrepare) const
{
	const auto systemctl = QStandardPaths::findExecutable(QStringLiteral("systemctl"));
	if (systemctl.isEmpty()) {
		setError(tr("Failed to find systemctl executable"));
		return -1;
	}

	QProcess process;
	process.setProgram(systemctl);

	QStringList args;
	args.reserve(extraArgs.size() + 4);
	if (_runAsUser)
		args.append(QStringLiteral("--user"));
	else
		args.append(QStringLiteral("--system"));
	args.append(QString::fromUtf8(command));
	if (!noPrepare) {
		args.append(serviceId());
		if (!_blocking)
			args.append(QStringLiteral("--no-block"));
	}
	args.append(extraArgs);
	process.setArguments(args);

	process.setStandardInputFile(QProcess::nullDevice());
	if (!outData)
		process.setStandardOutputFile(QProcess::nullDevice());
	process.setProcessChannelMode(QProcess::ForwardedErrorChannel);

	qCDebug(logControl) << "Executing" << process.program()
						<< process.arguments();
	process.start(QProcess::ReadOnly);
	if (process.waitForFinished(_blocking ? -1 : 2500)) {//non-blocking calls should finish within two seconds
		if (outData)
			*outData = process.readAllStandardOutput();

		if (process.exitStatus() == QProcess::NormalExit)
			return process.exitCode();
		else {
			setError(tr("systemctl crashed with error: %1").arg(process.errorString()));
			return 128 + process.error();
		}
	} else {
		setError(tr("systemctl did not exit in time"));
		return -1;
	}
}
