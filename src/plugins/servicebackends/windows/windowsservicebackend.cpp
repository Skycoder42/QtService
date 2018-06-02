/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** Copyright (c) 2018, Felix Barz
**
** This file is based on the the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "windowsservicebackend.h"
#include "windowsserviceplugin.h"
#include <iostream>
#include <QtCore/QFileInfo>
using namespace QtService;

#define SVCNAME const_cast<wchar_t*>(reinterpret_cast<const wchar_t*>(QCoreApplication::applicationName().utf16()))

QPointer<WindowsServiceBackend> WindowsServiceBackend::_backendInstance;

WindowsServiceBackend::WindowsServiceBackend(Service *service) :
	ServiceBackend{service}
{
	Q_ASSERT_X(!_backendInstance, Q_FUNC_INFO, "There can always be only a single backend!");
	_backendInstance = this;

	ZeroMemory(&_status, sizeof(_status));
	_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	_status.dwCurrentState = SERVICE_START_PENDING;
	_status.dwControlsAccepted = SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	_status.dwWin32ExitCode = NO_ERROR;
	_status.dwServiceSpecificExitCode = EXIT_SUCCESS;
	_status.dwWaitHint = 30000; //30 seconds should suffice
}

int WindowsServiceBackend::runService(int &argc, char **argv, int flags)
{
	Q_UNUSED(argc)
	Q_UNUSED(argv)
	qInstallMessageHandler(WindowsServiceBackend::winsvcMessageHandler);

	// if not set: get the app name from it's basename
	Q_ASSERT_X(!QCoreApplication::applicationName().isEmpty(), Q_FUNC_INFO, "QCoreApplication::applicationName must be set before starting a windows service!");

	// start handler and wait for service init
	SvcControlThread controlThread{this};
	controlThread.start();
	QMutexLocker lock(&_svcLock);
	if(!_startCondition.wait(&_svcLock, 20000))
		return EXIT_FAILURE;
	if(_status.dwWin32ExitCode != NO_ERROR)
		return EXIT_FAILURE;
	// generate "local" arguments
	auto sArgc = _svcArgs.size();
	QVector<char*> sArgv;
	sArgv.reserve(sArgc);
	for(auto &arg : _svcArgs)
		sArgv.append(arg.data());
	lock.unlock();

	// create and prepare the coreapp
	setStatus(SERVICE_START_PENDING);
	QCoreApplication app(sArgc, sArgv.data(), flags);
	app.installNativeEventFilter(new SvcEventFilter{});
	setStatus(SERVICE_START_PENDING);
	if(!preStartService())
		return EXIT_FAILURE; //TODO implement correctly
	setStatus(SERVICE_START_PENDING);
	connect(service(), &Service::started,
			this, &WindowsServiceBackend::onRunning);
	connect(service(), &Service::paused,
			this, &WindowsServiceBackend::onPaused);
	connect(service(), &Service::resumed,
			this, &WindowsServiceBackend::onRunning);
	connect(service(), &Service::stopped,
			qApp, &QCoreApplication::exit);

	_opTimer = new QTimer{this};
	connect(_opTimer, &QTimer::timeout,
			this, [this](){
		setStatus(_status.dwCurrentState);
	});
	_opTimer->setTimerType(Qt::CoarseTimer);
	_opTimer->setInterval(2000);
	_opTimer->start();
	setStatus(SERVICE_START_PENDING);

	lock.relock();
	_startCondition.wakeAll();
	lock.unlock();

	//execute the app
	_status.dwServiceSpecificExitCode = app.exec();
	if(_status.dwServiceSpecificExitCode != EXIT_SUCCESS)
		_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	setStatus(SERVICE_STOPPED);

	//cleanup
	if(controlThread.isRunning()) {
		controlThread.requestInterruption();
		if(!controlThread.wait(2000)) {
			controlThread.terminate();
			controlThread.wait(500);
		}
	}

	return _status.dwServiceSpecificExitCode;
}

void WindowsServiceBackend::quitService()
{
	setStatus(SERVICE_STOP_PENDING);
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, StopCommand));
}

void WindowsServiceBackend::reloadService()
{
	processServiceCommand(ReloadCommand);
}

void WindowsServiceBackend::onRunning()
{
	setStatus(SERVICE_RUNNING);
}

void WindowsServiceBackend::onPaused()
{
	setStatus(SERVICE_PAUSED);
}

void WindowsServiceBackend::setStatus(DWORD status)
{
	constexpr auto pendingFlags = SERVICE_START_PENDING | SERVICE_PAUSE_PENDING | SERVICE_CONTINUE_PENDING | SERVICE_STOP_PENDING;
	QMutexLocker lock(&_svcLock);
	if(status != _status.dwCurrentState) {
		_status.dwCurrentState = status;
		_status.dwCheckPoint = 0;
		if((status & pendingFlags) != 0)
			QMetaObject::invokeMethod(_opTimer, "start");
		else
			QMetaObject::invokeMethod(_opTimer, "stop");
	} else if((status & pendingFlags) != 0)
		_status.dwCheckPoint++;

	if(_statusHandle)
		SetServiceStatus(_statusHandle, &_status);
}

void WindowsServiceBackend::serviceMain(DWORD dwArgc, wchar_t **lpszArgv)
{
	Q_ASSERT(_backendInstance);

	_backendInstance->_statusHandle = RegisterServiceCtrlHandlerW(SVCNAME, WindowsServiceBackend::handler);
	Q_ASSERT(_backendInstance->_statusHandle);
	_backendInstance->setStatus(SERVICE_START_PENDING);

	// pass the arguments to the main thread and notifiy him
	QMutexLocker lock(&_backendInstance->_svcLock);
	_backendInstance->_svcArgs.clear();
	_backendInstance->_svcArgs.reserve(dwArgc);
	for(DWORD i = 0; i < dwArgc; i++)
		_backendInstance->_svcArgs.append(QString::fromWCharArray(lpszArgv[i]).toUtf8());

	_backendInstance->_startCondition.wakeAll();
	lock.unlock();

	// wait for the mainthread to finish startup, then register the service handler
	lock.relock();
	_backendInstance->_startCondition.wait(&_backendInstance->_svcLock);
	lock.unlock();

	// handle the start event
	_backendInstance->setStatus(SERVICE_START_PENDING);
	QMetaObject::invokeMethod(_backendInstance, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, StartCommand));
}

void WindowsServiceBackend::handler(DWORD dwOpcode)
{
	// could theoretically happen in a cleanup scenario?
	if(!_backendInstance)
		return;

	switch (dwOpcode) {
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		_backendInstance->quitService();
		break;
	case SERVICE_CONTROL_PAUSE:
		_backendInstance->setStatus(SERVICE_PAUSE_PENDING);
		QMetaObject::invokeMethod(_backendInstance, "processServiceCommand", Qt::QueuedConnection,
								  Q_ARG(QtService::ServiceBackend::ServiceCommand, PauseCommand));
		break;
	case SERVICE_CONTROL_CONTINUE:
		_backendInstance->setStatus(SERVICE_CONTINUE_PENDING);
		QMetaObject::invokeMethod(_backendInstance, "processServiceCommand", Qt::QueuedConnection,
								  Q_ARG(QtService::ServiceBackend::ServiceCommand, ResumeCommand));
		break;
	default:
		if (dwOpcode >= 128 && dwOpcode <= 255) {
			QMetaObject::invokeMethod(_backendInstance, "processServiceCallbackImpl", Qt::QueuedConnection,
									  Q_ARG(QByteArray, "command"),
									  Q_ARG(QVariantList, QVariantList{QVariant::fromValue(dwOpcode)}));
		}
		break;
	}
}

void WindowsServiceBackend::winsvcMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
	auto msg = qFormatLogMessage(type, context, message);
	auto h = RegisterEventSourceW(0, SVCNAME);
	if(h) {
		WORD wType;
		switch (type) {
		case QtFatalMsg:
		case QtCriticalMsg:
			wType = EVENTLOG_ERROR_TYPE;
			break;
		case QtWarningMsg:
			wType = EVENTLOG_WARNING_TYPE;
			break;
		case QtDebugMsg:
			wType = EVENTLOG_INFORMATION_TYPE;
			break;
		case QtInfoMsg:
			wType = EVENTLOG_SUCCESS;
			break;
		default:
			Q_UNREACHABLE();
			break;
		}

		auto contextStr = QStringLiteral("CATEGORY: %1\n"
										 "FILE: %2\n"
										 "FUNCTION: %3\n"
										 "LINE: %4\n"
										 "VERSION: %5")
				.arg(QString::fromUtf8(context.category),
					 QString::fromUtf8(context.file),
					 QString::fromUtf8(context.function))
				.arg(context.line)
				.arg(context.version)
				.toUtf8();

		auto msgStr = reinterpret_cast<const wchar_t *>(msg.utf16());
		ReportEvent(h, wType, 1, 1, NULL,
					1, contextStr.size(),
					&msgStr,
					contextStr.data());
		DeregisterEventSource(h);
	} else
		std::cerr << msg.toStdString() << std::endl;
}



WindowsServiceBackend::SvcControlThread::SvcControlThread(WindowsServiceBackend *backend) :
	QThread(),
	_backend{backend}
{
	setTerminationEnabled(true);
}

void WindowsServiceBackend::SvcControlThread::run()
{
	SERVICE_TABLE_ENTRYW st[2];
	st[0].lpServiceName = SVCNAME;
	st[0].lpServiceProc = WindowsServiceBackend::serviceMain;
	st[1].lpServiceName = 0;
	st[1].lpServiceProc = 0;

	if (StartServiceCtrlDispatcherW(st) == 0) { //blocking method
		qCCritical(logQtService).noquote() << qt_error_string(GetLastError());
		QMutexLocker lock(&_backend->_svcLock);
		_backend->_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		_backend->_startCondition.wakeAll();
	}
}



bool WindowsServiceBackend::SvcEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
	if(eventType == "windows_generic_MSG" ||
	   eventType == "windows_dispatcher_MSG") {
		auto winMessage = reinterpret_cast<MSG*>(message);
		if (winMessage->message == WM_ENDSESSION && (winMessage->lParam & ENDSESSION_LOGOFF)) {
			*result = TRUE;
			return true;
		}
	}
	return false;
}
