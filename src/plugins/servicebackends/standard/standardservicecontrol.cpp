#include "standardservicecontrol.h"
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
#include <QtService/private/logging_p.h>
using namespace QtService;

StandardServiceControl::StandardServiceControl(QString &&serviceId, QObject *parent) :
	ServiceControl{std::move(serviceId), parent},
	_statusLock{runtimeDir().absoluteFilePath(QStringLiteral("qstandard.lock"))}
{
	_statusLock.setStaleLockTime(std::numeric_limits<int>::max()); //disable stale locks
}

QString StandardServiceControl::backend() const
{
	return QStringLiteral("standard");
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
	if(_statusLock.tryLock()) {
		_statusLock.unlock();
		return ServiceStopped;
	} else if(_statusLock.error() == QLockFile::LockFailedError)
		return ServiceRunning;
	else {
		setError(QStringLiteral("Failed to access lockfile with error: %1").arg(_statusLock.error()));
		return ServiceStatusUnknown;
	}
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
		setError(QStringLiteral("Unabled to find executable for service with id \"%1\"").arg(serviceId()));
		return false;
	}

	QProcess svcProc;
	svcProc.setProgram(bin);
	svcProc.setArguments({QStringLiteral("--backend"), QStringLiteral("standard")});
	svcProc.setWorkingDirectory(QDir::rootPath());
	svcProc.setStandardInputFile(QProcess::nullDevice());
	svcProc.setStandardOutputFile(QProcess::nullDevice());
	svcProc.setStandardErrorFile(QProcess::nullDevice());
	qint64 pid = 0;
	auto ok = svcProc.startDetached(&pid);
	if(ok)
		qCDebug(logQtService) << "Started service process with PID" << pid;
	else
		setError(QStringLiteral("Failed to start service process with error: %1").arg(svcProc.errorString()));
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
		setError(QStringLiteral("Failed to get pid of running service"));
		return false;
	}
#ifdef Q_OS_WIN
	auto ok = false;
	auto hadConsole = FreeConsole();
	if(AttachConsole(static_cast<DWORD>(pid))) {
		if(SetConsoleCtrlHandler(nullptr, true)) {
			if(GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0)) {
				for(auto i = 0; i < 10; i++) {
					if(status() == ServiceRunning)
						QThread::msleep(500);
					else {
						ok = true;
						break;
					}
				}
				if(!ok)
					setError(QStringLiteral("Service did not stop yet"));
			} else
				setError(QStringLiteral("Failed to send stop signal with error: %1").arg(qt_error_string(GetLastError())));
			SetConsoleCtrlHandler(nullptr, false);
		} else
			setError(QStringLiteral("Failed to disable local console handler with error: %1").arg(qt_error_string(GetLastError())));
		FreeConsole();
	} else
		setError(QStringLiteral("Failed to attach to service console with error: %1").arg(qt_error_string(GetLastError())));
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

qint64 StandardServiceControl::getPid()
{
	qint64 pid = 0;
	QString hostname, appname;
	if(_statusLock.getLockInfo(&pid, &hostname, &appname))
		return pid;
	else
		return -1;
}
