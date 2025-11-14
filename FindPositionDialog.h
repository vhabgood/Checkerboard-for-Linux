#ifndef FINDPOSITIONDIALOG_H
#define FINDPOSITIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

namespace Ui {
class FindPositionDialog;
}

class FindPositionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindPositionDialog(QWidget *parent = nullptr);
    ~FindPositionDialog();

    QString getFenString() const;

signals:
    void findPositionRequested(const QString& fen);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::FindPositionDialog *ui;
    QLineEdit *fenLineEdit;
    QString m_fenString;
};

#endif // FINDPOSITIONDIALOG_H
