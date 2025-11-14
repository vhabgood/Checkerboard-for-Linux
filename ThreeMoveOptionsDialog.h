#ifndef THREEMOVEOPTIONSDIALOG_H
#define THREEMOVEOPTIONSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>

class ThreeMoveOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ThreeMoveOptionsDialog(int currentThreeMoveOption, QWidget *parent = nullptr);
    ~ThreeMoveOptionsDialog();

    int getSelectedThreeMoveOption() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    QComboBox *threeMoveOptionComboBox;
    int m_selectedThreeMoveOption;
};

#endif // THREEMOVEOPTIONSDIALOG_H
