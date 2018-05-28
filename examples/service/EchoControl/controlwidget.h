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

private:
	Ui::ControlWidget *ui;
	QtService::ServiceControl *_control = nullptr;

	void setStatus();
};

#endif // CONTROLWIDGET_H
