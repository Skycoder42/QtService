#include "controlwidget.h"
#include "ui_controlwidget.h"

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
	auto metaEnum = QMetaEnum::fromType<ServiceControl::SupportFlags>();
	ui->supportsLineEdit->setText(QString::fromUtf8(metaEnum.valueToKeys(static_cast<int>(_control->supportFlags()))));

	ui->loadButton->setVisible(false);
	ui->unloadButton->setVisible(true);

	ui->backendComboBox->setEnabled(false);
	ui->nameLineEdit->setEnabled(false);

	if(_control->supportFlags().testFlag(ServiceControl::SupportsBlockingNonBlocking)) {
		ui->bLockingCheckBox->setEnabled(true);
		ui->bLockingCheckBox->setChecked(_control->isBlocking());
	} else if(_control->supportFlags().testFlag(ServiceControl::SupportsBlocking))
		ui->bLockingCheckBox->setChecked(true);
	else if(_control->supportFlags().testFlag(ServiceControl::SupportsNonBlocking))
		ui->bLockingCheckBox->setChecked(false);
	else
		ui->bLockingCheckBox->setCheckState(Qt::PartiallyChecked);
	if(_control->supportFlags().testFlag(ServiceControl::SupportsEnableDisable))
		ui->enabledCheckBox->setEnabled(true);
	ui->enabledCheckBox->setChecked(_control->isEnabled());

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
	auto metaEnum = QMetaEnum::fromType<ServiceControl::ServiceStatus>();
	ui->statusLineEdit->setText(QString::fromUtf8(metaEnum.valueToKey(static_cast<int>(_control->status()))));
}
