#include "PriorityDialog.h"

PriorityDialog::PriorityDialog(int currentPriority, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Set Engine Priority"));

    priorityComboBox = new QComboBox(this);
    priorityComboBox->addItem(tr("Low"), 19); // Nice value for low priority
    priorityComboBox->addItem(tr("Normal"), 0); // Default priority
    priorityComboBox->addItem(tr("High"), -10); // Negative values for higher priority

    // Select current priority
    int index = priorityComboBox->findData(currentPriority);
    if (index != -1) {
        priorityComboBox->setCurrentIndex(index);
    } else {
        // If current priority doesn't match predefined, select Normal
        priorityComboBox->setCurrentIndex(priorityComboBox->findData(0));
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PriorityDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PriorityDialog::on_buttonBox_rejected);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(priorityComboBox);
    mainLayout->addWidget(buttonBox);
}

PriorityDialog::~PriorityDialog()
{
}

int PriorityDialog::getSelectedPriority() const
{
    return m_selectedPriority;
}

void PriorityDialog::on_buttonBox_accepted()
{
    m_selectedPriority = priorityComboBox->currentData().toInt();
    accept();
}

void PriorityDialog::on_buttonBox_rejected()
{
    reject();
}
