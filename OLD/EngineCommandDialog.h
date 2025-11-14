#ifndef ENGINECOMMANDDIALOG_H
#define ENGINECOMMANDDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class EngineCommandDialog;
}

class EngineCommandDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EngineCommandDialog(QWidget *parent = nullptr);
    ~EngineCommandDialog();

    QString getCommand() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::EngineCommandDialog *ui;
    QString m_command;
};

#endif // ENGINECOMMANDDIALOG_H
