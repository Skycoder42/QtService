#ifndef CONTROLHELPER_H
#define CONTROLHELPER_H

#include <QtService/ServiceControl>
#include <QAndroidServiceConnection>

class ControlHelper : public QObject
{
	Q_OBJECT

public:
	explicit ControlHelper(QtService::ServiceControl *parent = nullptr);

public slots:
	void startIntent(const QString &action);
	void bind();
	void unbind();

private:
	class Connection : public QAndroidServiceConnection
	{
	public:
		void onServiceConnected(const QString &name, const QAndroidBinder &serviceBinder) override;
		void onServiceDisconnected(const QString &name) override;

	private:
		void toast(const QString &message);
	} *_connection = nullptr;

	QtService::ServiceControl *_control;
};

#endif // CONTROLHELPER_H
