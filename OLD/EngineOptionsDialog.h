#ifndef ENGINEOPTIONSDIALOG_H
#define ENGINEOPTIONSDIALOG_H

#include <QDialog>
#include <QString>
#include <QVector>

namespace Ui {
class EngineOptionsDialog;
}

class EngineOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EngineOptionsDialog(const QString& engineName, const QString& currentOptions, QWidget *parent = nullptr);
    ~EngineOptionsDialog();

    QString getNewOptions() const;
    QString engineName() const;
    QString engineOptions() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::EngineOptionsDialog *ui;
    QString m_engineName;
    QString m_currentOptions;
    QString m_newOptions;
};

#endif // ENGINEOPTIONSDIALOG_H
