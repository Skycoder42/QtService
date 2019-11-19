#include <QCoreApplication>
#include "testservice.h"

int main(int argc, char *argv[])
{
#ifndef Q_CC_MSVC
	// WORKAROUND for windows service not beeing able to pass env vars
	if (qEnvironmentVariable("QT_PLUGIN_PATH").isEmpty()) {
		qputenv("QT_PLUGIN_PATH", QFileInfo{QString::fromUtf8(argv[0])}.dir().absolutePath().toUtf8());
		qDebug() << "QT_PLUGIN_PATH" << qEnvironmentVariable("QT_PLUGIN_PATH");
	}
	if (qEnvironmentVariable("QT_LOGGING_RULES").isEmpty()) {
		qputenv("QT_LOGGING_RULES", "qt.service.*.debug=true");
		qDebug() << "QT_LOGGING_RULES" << qEnvironmentVariable("QT_LOGGING_RULES");
	}
#endif
	qDebug() << "libraryPaths" << QCoreApplication::libraryPaths();

	TestService service{argc, argv};
	QCoreApplication::setApplicationName(QStringLiteral("testservice"));
	QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("de.skycoder42.qtservice.tests"));
	return service.exec();
}
