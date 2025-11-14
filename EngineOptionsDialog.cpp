#include "EngineOptionsDialog.h"
#include "ui_EngineOptionsDialog.h"
#include <QMessageBox>

EngineOptionsDialog::EngineOptionsDialog(const QString& engineName, const QString& currentOptions, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EngineOptionsDialog),
    m_engineName(engineName),
    m_currentOptions(currentOptions)
{
    ui->setupUi(this);
    setWindowTitle(tr("Engine Options for ") + m_engineName);
    ui->engineNameLabel->setText(m_engineName);
    ui->optionsPlainTextEdit->setPlainText(m_currentOptions);
}

EngineOptionsDialog::~EngineOptionsDialog()
{
    delete ui;
}

QString EngineOptionsDialog::getNewOptions() const
{
    return m_newOptions;
}

QString EngineOptionsDialog::engineName() const
{
    return m_engineName;
}

QString EngineOptionsDialog::engineOptions() const
{
    return m_newOptions; // Assuming m_newOptions holds the updated options
}

void EngineOptionsDialog::on_buttonBox_accepted()
{
    m_newOptions = ui->optionsPlainTextEdit->toPlainText();
    accept();
}

void EngineOptionsDialog::on_buttonBox_rejected()
{
    reject();
}
