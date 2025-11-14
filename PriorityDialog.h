#ifndef PRIORITYDIALOG_H
#define PRIORITYDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class PriorityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PriorityDialog(int currentPriority, QWidget *parent = nullptr);
    ~PriorityDialog();

    int getSelectedPriority() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    QComboBox *priorityComboBox;
    int m_selectedPriority;
};

#endif // PRIORITYDIALOG_H
