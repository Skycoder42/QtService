#include "controlwidget.h"
#include "ui_controlwidget.h"

#include <QMessageBox>
#include <QMetaEnum>
using namespace QtService;

ControlWidget::ControlWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ControlWidget)
{
	ui->setupUi(this);
	ui->backendComboBox->addItems(ServiceControl::listBackends());
	ui->unloadButton->hide();
	ui->statusButton->setDefaultAction(ui->actionReload);

	connect(ui->actionReload, &QAction::triggered,
			this, &ControlWidget::setStatus);
}

ControlWidget::~ControlWidget()
{
	delete ui;
}

void ControlWidget::on_loadButton_clicked()
{
	if(_control)
		_control->deleteLater();
	_control = ServiceControl::create(ui->backendComboBox->currentText(),
									  ui->nameLineEdit->text(),
									  this);
	connect(_control, &ServiceControl::errorChanged,
			this, [this](const QString &error){
		QMessageBox::critical(this, tr("Error"), error);
	});
	if(!_control->serviceExists()) {
		QMessageBox::critical(this,
							  tr("Error"),
							  tr("Unable to find a service of name \"%1\" with backend \"%2\"")
							  .arg(_control->serviceId(), _control->backend()));
		_control->deleteLater();
		_control = nullptr;
		return;
	}

	auto metaEnum = QMetaEnum::fromType<ServiceControl::SupportFlags>();
	ui->supportsLineEdit->setText(QString::fromUtf8(metaEnum.valueToKeys(static_cast<int>(_control->supportFlags()))));

	ui->loadButton->setVisible(false);
	ui->unloadButton->setVisible(true);

	ui->backendComboBox->setEnabled(false);
	ui->nameLineEdit->setEnabled(false);

	if(_control->supportFlags().testFlag(ServiceControl::SupportsSetBlocking)) {
		ui->bLockingCheckBox->setEnabled(true);
		ui->bLockingCheckBox->setChecked(_control->blocking() == ServiceControl::Blocking);
	} else if(_control->blocking() == ServiceControl::Blocking)
		ui->bLockingCheckBox->setChecked(true);
	else if(_control->blocking() == ServiceControl::NonBlocking)
		ui->bLockingCheckBox->setChecked(false);
	else
		ui->bLockingCheckBox->setCheckState(Qt::PartiallyChecked);

	if(_control->supportFlags().testFlag(ServiceControl::SupportsGetAutostart))
		ui->enabledCheckBox->setChecked(_control->isAutostartEnabled());
	if(_control->supportFlags().testFlag(ServiceControl::SupportsSetAutostart))
		ui->enabledCheckBox->setEnabled(true);

	if(_control->supportFlags().testFlag(ServiceControl::SupportsStatus)) {
		ui->actionReload->setEnabled(true);
		setStatus();
	}

	if(_control->supportFlags().testFlag(ServiceControl::SupportsStart))
		ui->startButton->setEnabled(true);
	if(_control->supportFlags().testFlag(ServiceControl::SupportsStop))
		ui->stopButton->setEnabled(true);
	if(_control->supportFlags().testFlag(ServiceControl::SupportsPause))
		ui->pauseButton->setEnabled(true);
	if(_control->supportFlags().testFlag(ServiceControl::SupportsResume))
		ui->resumeButton->setEnabled(true);
	if(_control->supportFlags().testFlag(ServiceControl::SupportsReload))
		ui->reloadButton->setEnabled(true);
}

void ControlWidget::on_unloadButton_clicked()
{
	if(_control)
		_control->deleteLater();
	_control = nullptr;

	ui->loadButton->setVisible(true);
	ui->unloadButton->setVisible(false);

	ui->backendComboBox->setEnabled(true);
	ui->nameLineEdit->setEnabled(true);

	ui->bLockingCheckBox->setEnabled(false);
	ui->bLockingCheckBox->setChecked(false);
	ui->enabledCheckBox->setEnabled(false);
	ui->enabledCheckBox->setChecked(false);

	ui->supportsLineEdit->clear();
	ui->actionReload->setEnabled(false);
	ui->statusLineEdit->clear();

	ui->startButton->setEnabled(false);
	ui->stopButton->setEnabled(false);
	ui->pauseButton->setEnabled(false);
	ui->resumeButton->setEnabled(false);
	ui->reloadButton->setEnabled(false);
}

void ControlWidget::setStatus()
{
	if(!_control)
		return;
	auto metaEnum = QMetaEnum::fromType<ServiceControl::ServiceStatus>();
	ui->statusLineEdit->setText(QString::fromUtf8(metaEnum.valueToKey(static_cast<int>(_control->status()))));
	if(_control->supportFlags().testFlag(ServiceControl::SupportsGetAutostart))
		ui->enabledCheckBox->setChecked(_control->isAutostartEnabled());
}

void ControlWidget::on_bLockingCheckBox_clicked(bool checked)
{
	if(!_control)
		return;
	_control->setBlocking(checked);
}

void ControlWidget::on_enabledCheckBox_clicked(bool checked)
{
	if(!_control)
		return;
	if(checked)
		_control->enableAutostart();
	else
		_control->disableAutostart();
}

void ControlWidget::on_startButton_clicked()
{
	if(!_control)
		return;
	_control->start();
}

void ControlWidget::on_stopButton_clicked()
{
	if(!_control)
		return;
	_control->stop();
}

void ControlWidget::on_pauseButton_clicked()
{
	if(!_control)
		return;
	_control->pause();
}

void ControlWidget::on_resumeButton_clicked()
{
	if(!_control)
		return;
	_control->resume();
}

void ControlWidget::on_reloadButton_clicked()
{
	if(!_control)
		return;
	_control->reload();
}
