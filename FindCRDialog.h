#ifndef FINDCRDIALOG_H
#define FINDCRDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

namespace Ui {
class FindCRDialog;
}

class FindCRDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindCRDialog(QWidget *parent = nullptr);
    ~FindCRDialog();

    QString getSearchString() const;

signals:
    void findCRRequested(const QString& searchString);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::FindCRDialog *ui;
    QLineEdit *searchLineEdit;
    QString m_searchString;
};

#endif // FINDCRDIALOG_H
