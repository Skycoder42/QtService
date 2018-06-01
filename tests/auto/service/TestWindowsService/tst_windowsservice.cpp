#include <QString>
#include <QtTest/QtTest>
#include <QCoreApplication>
#include <basicservicetest.h>
#include <qt_windows.h>
using namespace QtService;

class TestWindowsService : public BasicServiceTest
{
	Q_OBJECT

protected:
	QString backend() override;
	QString name() override;
	void init() override;
	void cleanup() override;
	void testCustomImpl() override;

private:
	SC_HANDLE _manager = nullptr;
};

QString TestWindowsService::backend()
{
	return QStringLiteral("windows");
}

QString TestWindowsService::name()
{
	return QStringLiteral("testservice");
}

void TestWindowsService::init()
{
#ifdef QT_NO_DEBUG
	QString svcPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/release/testservice.exe");
#else
	QString svcPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/debug/testservice.exe");
#endif
	svcPath = QStringLiteral("\"%1\" --backend windows").arg(svcPath);

	_manager = OpenSCManagerW(nullptr, nullptr,
							  SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE | STANDARD_RIGHTS_REQUIRED);
	QVERIFY2(_manager, qUtf8Printable(qt_error_string(GetLastError())));
	auto handle = CreateServiceW(_manager,
								 reinterpret_cast<const wchar_t*>(name().utf16()),
								 L"Test Service",
								 SERVICE_CHANGE_CONFIG,
								 SERVICE_WIN32_OWN_PROCESS,
								 SERVICE_DEMAND_START,
								 SERVICE_ERROR_IGNORE,
								 reinterpret_cast<const wchar_t*>(svcPath.utf16()),
								 nullptr,
								 nullptr,
								 nullptr,
								 nullptr,
								 nullptr);
	QVERIFY2(handle, qUtf8Printable(qt_error_string(GetLastError())));

	CloseServiceHandle(handle);
}

void TestWindowsService::cleanup()
{
	if(!_manager)
		return;

	auto handle = OpenServiceW(_manager,
							   reinterpret_cast<const wchar_t*>(name().utf16()),
							   DELETE);
	QVERIFY2(handle, qUtf8Printable(qt_error_string(GetLastError())));
	QVERIFY2(DeleteService(handle), qUtf8Printable(qt_error_string(GetLastError())));

	CloseServiceHandle(handle);
	CloseServiceHandle(_manager);
	_manager = nullptr;
}

void TestWindowsService::testCustomImpl()
{
	testFeature(ServiceControl::SupportsStatus);
	QCOMPARE(control->status(), ServiceControl::ServiceRunning);

	testFeature(ServiceControl::SupportsCustomCommands);
	QVERIFY2(control->callCommand<bool>("command", 142), qUtf8Printable(control->error()));

	QByteArray msg;
	QVariantList args;
	READ_LOOP(msg >> args);
	QCOMPARE(msg, QByteArray{"command"});
	QCOMPARE(args, QVariantList{142});

	testFeature(ServiceControl::SupportsStatus);
	QCOMPARE(control->status(), ServiceControl::ServiceRunning);
}

QTEST_MAIN(TestWindowsService)

#include "tst_windowsservice.moc"
