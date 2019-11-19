#include <QCoreApplication>
#include "testservice.h"

int main(int argc, char *argv[])
{
	// WORKAROUND for CI bug
	qDebug() << "libraryPaths" << QCoreApplication::libraryPaths();
	if (QCoreApplication::libraryPaths().isEmpty()) {
		qputenv("QT_PLUGIN_PATH", QFileInfo{QString::fromUtf8(argv[0])}.dir().absolutePath().toUtf8());
		qDebug() << "QT_PLUGIN_PATH" << qEnvironmentVariable("QT_PLUGIN_PATH");
		qDebug() << "libraryPaths" << QCoreApplication::libraryPaths();
	}

	TestService service{argc, argv};
	QCoreApplication::setApplicationName(QStringLiteral("testservice"));
	QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("de.skycoder42.qtservice.tests"));
	return service.exec();
}
