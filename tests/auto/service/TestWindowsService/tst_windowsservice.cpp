#include <QString>
#include <QtTest/QtTest>
#include <QCoreApplication>
#include <basicservicetest.h>
#include <qt_windows.h>
using namespace QtService;

#ifdef QT_NO_DEBUG
#define LIB(x) QStringLiteral(x ".dll")
#else
#define LIB(x) QStringLiteral(x "d.dll")
#endif

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
	QDir svcDir{QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/release")};
#else
	QDir svcDir{QCoreApplication::applicationDirPath() + QStringLiteral("/../../TestService/debug")};
#endif
	QVERIFY(svcDir.exists(QStringLiteral("testservice.exe")));
	auto svcPath = QStringLiteral("\"%1\" --backend windows").arg(QDir::toNativeSeparators(svcDir.absoluteFilePath(QStringLiteral("testservice.exe"))));

	// copy Qt libs
	QDir qtLibDir;
	if (const auto lDir = QLibraryInfo::location(QLibraryInfo::BinariesPath); lDir.isEmpty()) {
		qDebug() << "Fallback Qt path:" << QT_LIB_DIR;
		qtLibDir.setPath(QStringLiteral(QT_LIB_DIR));
	} else {
		qDebug() << "Primary Qt path:" << lDir;
		qtLibDir.setPath(lDir);
	}
	qDebug() << "qtLibDir" << qtLibDir;
	for(const auto &baseLib : {LIB("Qt5Core"), LIB("Qt5Network")}) {
		if(!svcDir.exists(baseLib))
			QVERIFY(QFile::copy(qtLibDir.absoluteFilePath(baseLib), svcDir.absoluteFilePath(baseLib)));
	}

	// copy svc lib
	auto svcLib = LIB("Qt5Service");
	QDir bLibDir{QCoreApplication::applicationDirPath() + QStringLiteral("/../../../../../lib")};
	QVERIFY(bLibDir.exists(svcLib));
	svcDir.remove(svcLib);
	QVERIFY(QFile::copy(bLibDir.absoluteFilePath(svcLib), svcDir.absoluteFilePath(svcLib)));

	// add plugins to Qt
	QDir qtPlgDir{QLibraryInfo::location(QLibraryInfo::PluginsPath)};
	qDebug() << "qtPlgDir" << qtPlgDir;
	QDir bPlgDir{QCoreApplication::applicationDirPath() + QStringLiteral("/../../../../../plugins/servicebackends")};
	auto plgNew = QStringLiteral("servicebackends");
	auto plgOld = QStringLiteral("servicebackends.old");
	auto realPlg = LIB("qwindows");
	QVERIFY(bPlgDir.exists(realPlg));
	qtPlgDir.rename(plgNew, plgOld);
	QVERIFY(qtPlgDir.mkdir(plgNew));
	QVERIFY(qtPlgDir.cd(plgNew));
	QVERIFY(QFile::copy(bPlgDir.absoluteFilePath(realPlg), qtPlgDir.absoluteFilePath(realPlg)));

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
	if(_manager) {
		auto handle = OpenServiceW(_manager,
								   reinterpret_cast<const wchar_t*>(name().utf16()),
								   DELETE);
		QVERIFY2(handle, qUtf8Printable(qt_error_string(GetLastError())));
		QVERIFY2(DeleteService(handle), qUtf8Printable(qt_error_string(GetLastError())));

		CloseServiceHandle(handle);
		CloseServiceHandle(_manager);
		_manager = nullptr;
	}

	QDir qtPlgDir{QLibraryInfo::location(QLibraryInfo::PluginsPath)};
	auto plgOld = QStringLiteral("servicebackends.old");
	if(qtPlgDir.exists(plgOld)) {
		auto plgNew = QStringLiteral("servicebackends");
		auto svcPlgDir = qtPlgDir;
		QVERIFY(svcPlgDir.cd(plgNew));
		QVERIFY(svcPlgDir.removeRecursively());
		QVERIFY(qtPlgDir.rename(plgOld, plgNew));
	}
}

void TestWindowsService::testCustomImpl()
{
	testFeature(ServiceControl::SupportFlag::Status);
	QCOMPARE(control->status(), ServiceControl::Status::Running);

	testFeature(ServiceControl::SupportFlag::CustomCommands);
	QVERIFY2(control->callCommand<bool>("command", 142), qUtf8Printable(control->error()));

	QByteArray msg;
	QVariantList args;
	READ_LOOP(msg >> args);
	QCOMPARE(msg, QByteArray{"command"});
	QCOMPARE(args, QVariantList{142});

	testFeature(ServiceControl::SupportFlag::Status);
	QCOMPARE(control->status(), ServiceControl::Status::Running);
}

QTEST_MAIN(TestWindowsService)

#include "tst_windowsservice.moc"
