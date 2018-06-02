#include "windowsservicecontrol.h"
#include <QtService/private/logging_p.h>
using namespace QtService;

WindowsServiceControl::WindowsServiceControl(QString &&serviceId, QObject *parent) :
	ServiceControl{std::move(serviceId), parent}
{
	//try to open the handle
	_manager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
	if(!_manager)
		setWinError(tr("Failed to get acces to service manager with error: %1"));
}

QString WindowsServiceControl::backend() const
{
	return QStringLiteral("windows");
}

ServiceControl::SupportFlags WindowsServiceControl::supportFlags() const
{
	return SupportsStartStop |
			SupportsPauseResume |
			SupportsAutostart |
			SupportsNonBlocking |
			SupportsStatus |
			SupportsCustomCommands;
}

bool WindowsServiceControl::serviceExists() const
{
	return status() != ServiceStatusUnknown;
}

ServiceControl::ServiceStatus WindowsServiceControl::status() const
{
	HandleHolder handle {svcHandle(SERVICE_QUERY_STATUS)};
	if(!handle)
		return ServiceStatusUnknown;

	SERVICE_STATUS status;
	ZeroMemory(&status, sizeof(status));
	if(!QueryServiceStatus(handle, &status)) {
		setWinError(tr("Failed to query service status with error: %1"));
		return ServiceStatusUnknown;
	}

	switch(status.dwCurrentState) {
	case SERVICE_START_PENDING:
		return ServiceStarting;
	case SERVICE_RUNNING:
		return ServiceRunning;
	case SERVICE_STOP_PENDING:
		return ServiceStopping;
	case SERVICE_STOPPED:
		return ServiceStopped;
	case SERVICE_PAUSE_PENDING:
		return ServicePausing;
	case SERVICE_PAUSED:
		return ServicePaused;
	case SERVICE_CONTINUE_PENDING:
		return ServiceResuming;
	default:
		return ServiceStatusUnknown;
	}
}

bool WindowsServiceControl::isAutostartEnabled() const
{
	HandleHolder handle {svcHandle(SERVICE_QUERY_CONFIG)};
	if(!handle)
		return false;

	DWORD sizeNeeded = 0;
	QueryServiceConfigW(handle, nullptr, 0, &sizeNeeded);
	QByteArray cData{static_cast<int>(sizeNeeded), '\0'};
	auto config = reinterpret_cast<LPQUERY_SERVICE_CONFIGW>(cData.data());
	if(!QueryServiceConfigW(handle, config, cData.size(), &sizeNeeded)) {
		setWinError(tr("Failed to query service status with error: %1"));
		return false;
	}

	switch(config->dwStartType) {
	case SERVICE_AUTO_START:
	case SERVICE_BOOT_START:
	case SERVICE_SYSTEM_START:
		return true;
	default:
		return false;
	}
}

QVariant WindowsServiceControl::callGenericCommand(const QByteArray &kind, const QVariantList &args)
{
	if(kind == "command") {
		if(args.size() != 1) {
			setError(tr("The command must be called with a single integer [128,255] as argument"));
			return {};
		}
		auto ok = false;
		DWORD cmd = args.first().toUInt(&ok);
		if(!ok || cmd < 128 || cmd > 255) {
			setError(tr("The command must be called with a single integer [128,255] as argument"));
			return {};
		}

		HandleHolder handle {svcHandle(SERVICE_USER_DEFINED_CONTROL)};
		if(!handle)
			return false;
		SERVICE_STATUS status;
		if(ControlService(handle, cmd, &status))
			return true;
		else {
			setWinError(tr("Failed to send command %1 to service with error: %2").arg(cmd));
			return false;
		}
	} else
		return ServiceControl::callGenericCommand(kind, args);
}

bool WindowsServiceControl::start()
{
	HandleHolder handle {svcHandle(SERVICE_START)};
	if(!handle)
		return false;
	if(StartServiceW(handle, 0, nullptr))
		return true;
	else {
		auto code = GetLastError();
		if(code == ERROR_SERVICE_ALREADY_RUNNING)
			return true;
		else {
			setWinError(tr("Failed to start service with error: %1"));
			return false;
		}
	}
}

bool WindowsServiceControl::stop()
{
	HandleHolder handle {svcHandle(SERVICE_STOP)};
	if(!handle)
		return false;
	SERVICE_STATUS status;
	if(ControlService(handle, SERVICE_CONTROL_STOP, &status))
		return true;
	else {
		auto code = GetLastError();
		if(code == ERROR_SERVICE_NOT_ACTIVE)
			return true;
		else {
			setWinError(tr("Failed to stop service with error: %1"));
			return false;
		}
	}
}

bool WindowsServiceControl::pause()
{
	HandleHolder handle {svcHandle(SERVICE_PAUSE_CONTINUE)};
	if(!handle)
		return false;
	SERVICE_STATUS status;
	if(ControlService(handle, SERVICE_CONTROL_PAUSE, &status))
		return true;
	else {
		setWinError(tr("Failed to pause service with error: %1"));
		return false;
	}
}

bool WindowsServiceControl::resume()
{
	HandleHolder handle {svcHandle(SERVICE_PAUSE_CONTINUE)};
	if(!handle)
		return false;
	SERVICE_STATUS status;
	if(ControlService(handle, SERVICE_CONTROL_CONTINUE, &status))
		return true;
	else {
		setWinError(tr("Failed to resume service with error: %1"));
		return false;
	}
}

bool WindowsServiceControl::enableAutostart()
{
	HandleHolder handle {svcHandle(SERVICE_CHANGE_CONFIG)};
	if(!handle)
		return false;

	if(ChangeServiceConfigW(handle,
							 SERVICE_NO_CHANGE,
							 SERVICE_AUTO_START, // only line that actually changes stuff
							 SERVICE_NO_CHANGE,
							 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)) {
		return true;
	} else {
		setWinError(tr("Failed to enable autostart with error: %1"));
		return false;
	}
}

bool WindowsServiceControl::disableAutostart()
{
	HandleHolder handle {svcHandle(SERVICE_CHANGE_CONFIG)};
	if(!handle)
		return false;

	if(ChangeServiceConfigW(handle,
							 SERVICE_NO_CHANGE,
							 SERVICE_DEMAND_START, // only line that actually changes stuff
							 SERVICE_NO_CHANGE,
							 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)) {
		return true;
	} else {
		setWinError(tr("Failed to enable autostart with error: %1"));
		return false;
	}
}

SC_HANDLE WindowsServiceControl::svcHandle(DWORD permissions) const
{
	if(!_manager)
		return nullptr; // error set in constructor

	auto handle = OpenServiceW(_manager,
							   reinterpret_cast<const wchar_t*>(serviceId().utf16()),
							   permissions);
	if(!handle)
		setWinError(tr("Failed to get access to service with error: %1"));
	return handle;
}

void WindowsServiceControl::setWinError(const QString &baseMsg) const
{
	setError(baseMsg.arg(qt_error_string(GetLastError())));
}



WindowsServiceControl::HandleHolder::HandleHolder(SC_HANDLE handle) :
	_handle(std::move(handle))
{}

WindowsServiceControl::HandleHolder &WindowsServiceControl::HandleHolder::operator=(SC_HANDLE handle)
{
	if(_handle)
		CloseServiceHandle(handle);
	_handle = std::move(handle);
	return *this;
}

WindowsServiceControl::HandleHolder::~HandleHolder()
{
	if(_handle)
		CloseServiceHandle(_handle);
}

bool WindowsServiceControl::HandleHolder::operator!() const
{
	return !_handle;
}

WindowsServiceControl::HandleHolder::operator SC_HANDLE() const
{
	return _handle;
}
