#include "standardservicecontrol.h"
#include "standardserviceplugin.h"
#include <QtCore/QStandardPaths>
#if QT_CONFIG(process)
#include <QtCore/QProcess>
#endif
#ifdef Q_OS_WIN
#include <QtCore/QThread>
#include <qt_windows.h>
#else
#include <csignal>
#endif
using namespace QtService;

StandardServiceControl::StandardServiceControl(bool debugMode, QString &&serviceId, QObject *parent) :
	ServiceControl{std::move(serviceId), parent},
	_debugMode{debugMode}
{}

QString StandardServiceControl::backend() const
{
	return _debugMode ? QStringLiteral("debug") : QStringLiteral("standard");
}

ServiceControl::SupportFlags StandardServiceControl::supportFlags() const
{
	SupportFlags flags = SupportsStatus | SupportsStop;
#if QT_CONFIG(process)
	flags |= SupportsStart;
#endif
	return flags;
}

bool StandardServiceControl::serviceExists() const
{
	return !QStandardPaths::findExecutable(serviceId()).isEmpty();
}

ServiceControl::ServiceStatus StandardServiceControl::status() const
{
	const auto lock = statusLock();
	if(lock->tryLock()) {
		lock->unlock();
		return ServiceStopped;
	} else if(lock->error() == QLockFile::LockFailedError)
		return ServiceRunning;
	else {
		setError(tr("Failed to access lockfile with error: %1").arg(lock->error()));
		return ServiceStatusUnknown;
	}
}

ServiceControl::BlockMode StandardServiceControl::blocking() const
{
	return NonBlocking;
}

QVariant StandardServiceControl::callGenericCommand(const QByteArray &kind, const QVariantList &args)
{
	Q_UNUSED(args)
	if(kind == "getPid")
		return getPid();
	else
		return {};
}

bool StandardServiceControl::start()
{
#if QT_CONFIG(process)
	if(status() == ServiceRunning) {
		qCDebug(logQtService) << "Service already running with PID" << getPid();
		return true;
	}

	auto bin = QStandardPaths::findExecutable(serviceId());
	if(bin.isEmpty()) {
		setError(tr("Unabled to find executable for service with id \"%1\"").arg(serviceId()));
		return false;
	}

	const auto prepareProc = [&](QProcess *svcProc){
		svcProc->setProgram(bin);
		svcProc->setArguments({QStringLiteral("--backend"), backend()});
		svcProc->setWorkingDirectory(QDir::rootPath());
	};

	auto ok = false;
	qint64 pid = 0;
	QString errorString;
	if(_debugMode) {
		auto svcProc = new QProcess{nullptr}; //detached instance
		connect(svcProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
				svcProc, &QProcess::deleteLater);
		prepareProc(svcProc);
		svcProc->setProcessChannelMode(QProcess::ForwardedChannels);
		svcProc->setInputChannelMode(QProcess::ForwardedInputChannel);
		svcProc->start();
		ok = svcProc->waitForStarted();
		if(ok)
			pid = svcProc->processId();
		else
			errorString = svcProc->errorString();
	} else {
		QProcess svcProc;
		prepareProc(&svcProc);
		svcProc.setStandardInputFile(QProcess::nullDevice());
		svcProc.setStandardOutputFile(QProcess::nullDevice());
		svcProc.setStandardErrorFile(QProcess::nullDevice());
		ok = svcProc.startDetached(&pid);
		if(!ok)
			errorString = svcProc.errorString();
	}

	if(ok)
		qCDebug(logQtService) << "Started service process with PID" << pid;
	else
		setError(tr("Failed to start service process with error: %1").arg(errorString));
	return ok;
#else
	return ServiceControl::start();
#endif
}

bool StandardServiceControl::stop()
{
	if(status() == ServiceStopped) {
		qCDebug(logQtService) << "Service already stopped ";
		return true;
	}

	auto pid = getPid();
	if(pid == -1) {
		setError(tr("Failed to get pid of running service"));
		return false;
	}
#ifdef Q_OS_WIN
	auto ok = false;
	auto hadConsole = FreeConsole();
	if(AttachConsole(static_cast<DWORD>(pid))) {
		if(SetConsoleCtrlHandler(nullptr, true)) {
			if(!GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0))
				setError(tr("Failed to send stop signal with error: %1").arg(qt_error_string(GetLastError())));
			SetConsoleCtrlHandler(nullptr, false);
		} else
			setError(tr("Failed to disable local console handler with error: %1").arg(qt_error_string(GetLastError())));
		FreeConsole();
	} else
		setError(tr("Failed to attach to service console with error: %1").arg(qt_error_string(GetLastError())));
	if(hadConsole)
		AllocConsole();
	return ok;
#else
	return kill(static_cast<pid_t>(pid), SIGTERM) == 0;
#endif
}

QString StandardServiceControl::serviceName() const
{
	QFileInfo info{serviceId()};
	if(info.isExecutable())
		return QFileInfo{serviceId()}.completeBaseName();
	else
		return serviceId().split(QLatin1Char('/'), QString::SkipEmptyParts).last();
}

QSharedPointer<QLockFile> StandardServiceControl::statusLock() const
{
	const auto lock = QSharedPointer<QLockFile>::create(runtimeDir().absoluteFilePath(QStringLiteral("qstandard.lock")));
	lock->setStaleLockTime(std::numeric_limits<int>::max()); //disable stale locks
	return lock;
}

qint64 StandardServiceControl::getPid()
{
	qint64 pid = 0;
	QString hostname, appname;
	if(statusLock()->getLockInfo(&pid, &hostname, &appname))
		return pid;
	else
		return -1;
}
