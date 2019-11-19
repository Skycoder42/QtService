#include <QCoreApplication>
#include "testservice.h"

int main(int argc, char *argv[])
{
#ifdef Q_CC_MSVC
	// WORKAROUND for CI bug
	if (qEnvironmentVariable("QT_PLUGIN_PATH").isEmpty()) {
		qputenv("QT_PLUGIN_PATH", QFileInfo{QString::fromUtf8(argv[0])}.dir().absolutePath().toUtf8());
		qDebug() << "QT_PLUGIN_PATH" << qEnvironmentVariable("QT_PLUGIN_PATH");
	}
#endif
	qDebug() << "libraryPaths" << QCoreApplication::libraryPaths();

	TestService service{argc, argv};
	QCoreApplication::setApplicationName(QStringLiteral("testservice"));
	QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("de.skycoder42.qtservice.tests"));
	return service.exec();
}
