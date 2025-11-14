#include "EngineCommandDialog.h"
#include "ui_EngineCommandDialog.h"

EngineCommandDialog::EngineCommandDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EngineCommandDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Engine Command"));
}

EngineCommandDialog::~EngineCommandDialog()
{
    delete ui;
}

QString EngineCommandDialog::getCommand() const
{
    return m_command;
}

void EngineCommandDialog::on_buttonBox_accepted()
{
    m_command = ui->commandLineEdit->text();
    accept();
}

void EngineCommandDialog::on_buttonBox_rejected()
{
    reject();
}
