#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <QtWidgets/QWidget>
#include <QtService/ServiceControl>

namespace Ui {
class ControlWidget;
}

class ControlWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ControlWidget(QWidget *parent = nullptr);
	~ControlWidget() override;

private Q_SLOTS:
	void on_loadButton_clicked();
	void on_unloadButton_clicked();

	void on_bLockingCheckBox_clicked(bool checked);
	void on_enabledCheckBox_clicked(bool checked);

	void on_startButton_clicked();
	void on_stopButton_clicked();
	void on_pauseButton_clicked();
	void on_resumeButton_clicked();
	void on_reloadButton_clicked();

private:
	Ui::ControlWidget *ui;
	QtService::ServiceControl *_control = nullptr;

	void setStatus();
};

#endif // CONTROLWIDGET_H
