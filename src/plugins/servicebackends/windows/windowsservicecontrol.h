#ifndef WINDOWSSERVICECONTROL_H
#define WINDOWSSERVICECONTROL_H

#include <QtCore/QLoggingCategory>

#include <QtService/ServiceControl>

#include <QtCore/qt_windows.h>

class WindowsServiceControl : public QtService::ServiceControl
{
	Q_OBJECT

public:
	explicit WindowsServiceControl(QString &&serviceId, QObject *parent = nullptr);

	QString backend() const override;
	SupportFlags supportFlags() const override;
	bool serviceExists() const override;
	Status status() const override;
	bool isAutostartEnabled() const override;
	bool isEnabled() const override;
	BlockMode blocking() const override;
	QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args) override;

public Q_SLOTS:
	bool start() override;
	bool stop() override;
	bool pause() override;
	bool resume() override;
	bool enableAutostart() override;
	bool disableAutostart() override;
	bool setEnabled(bool enabled) override;

private:
	class HandleHolder {
		Q_DISABLE_COPY(HandleHolder)
	public:
		HandleHolder(SC_HANDLE handle = nullptr);
		HandleHolder(HandleHolder &&other) noexcept;
		HandleHolder &operator=(SC_HANDLE handle);
		HandleHolder &operator=(HandleHolder &&other) noexcept;
		~HandleHolder();

		bool operator!() const;
		operator SC_HANDLE() const;
	private:
		SC_HANDLE _handle = nullptr;
	};

	HandleHolder _manager;

	HandleHolder svcHandle(DWORD permissions) const;
	void setWinError(const QString &baseMsg) const;
};

Q_DECLARE_LOGGING_CATEGORY(logControl)

#endif // WINDOWSSERVICECONTROL_H
