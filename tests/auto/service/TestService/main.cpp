#include <QCoreApplication>
#include "testservice.h"

int main(int argc, char *argv[])
{
#ifdef Q_CC_MSVC
	QFile envFile{QString::fromUtf8(argv[0]) + QStringLiteral(".env")};
	if (envFile.exists()) {
		if (!envFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qCritical() << "Failed to read env file" << envFile.fileName()
						<< "with error:" << qUtf8Printable(envFile.errorString());
			return EXIT_FAILURE;
		}

		while (!envFile.atEnd()) {
			const auto lineStr = envFile.readLine().trimmed();
			if (lineStr.isEmpty())
				continue;
			const auto envLine = lineStr.split('=');
			if (envFile.size() != 2) {
				qWarning() << "Invalid env line:" << envLine;
				continue;
			}
			if (qputenv(envLine[0], envLine[1])) {
				qInfo().nospace() << "Set env var: " << envLine[0]
						<< "=" << envLine[1];
			} else {
				qWarning() << "Failed to set env var" << envLine[0]
						   << "to" << envLine[1];
			}
		}
		envFile.close();
	}
#endif
	qDebug() << "libraryPaths" << QCoreApplication::libraryPaths();

	TestService service{argc, argv};
	QCoreApplication::setApplicationName(QStringLiteral("testservice"));
	QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("de.skycoder42.qtservice.tests"));
	return service.exec();
}
