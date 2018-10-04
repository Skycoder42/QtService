#ifndef TESTSERVICE_H
#define TESTSERVICE_H

#include <QtService/Service>
#include <QAndroidIntent>
#include <QAndroidBinder>

class TestService : public QtService::Service
{
	Q_OBJECT

public:
	explicit TestService(int &argc, char **argv);

protected:
	CommandResult onStart() override;
	CommandResult onStop(int &exitCode) override;

	int onStartCommand(const QAndroidIntent &intent, int flags, int startId);
	QAndroidBinder *onBind(const QAndroidIntent &intent);

private:
	void doStartNotify();
};

Q_DECLARE_METATYPE(QAndroidIntent)
Q_DECLARE_METATYPE(QAndroidBinder*)

#endif // TESTSERVICE_H
