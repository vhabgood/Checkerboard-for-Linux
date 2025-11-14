#include "ThreeMoveOptionsDialog.h"

ThreeMoveOptionsDialog::ThreeMoveOptionsDialog(int currentThreeMoveOption, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("3-Move Game Options"));

    QLabel *label = new QLabel(tr("Select 3-Move Option:"), this);
    threeMoveOptionComboBox = new QComboBox(this);
    threeMoveOptionComboBox->addItem(tr("Random 3-Move Opening"), 0);
    threeMoveOptionComboBox->addItem(tr("Specific 3-Move Opening"), 1);
    // Add more options as needed

    // Select current option
    int index = threeMoveOptionComboBox->findData(currentThreeMoveOption);
    if (index != -1) {
        threeMoveOptionComboBox->setCurrentIndex(index);
    } else {
        threeMoveOptionComboBox->setCurrentIndex(0); // Default to Random
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ThreeMoveOptionsDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ThreeMoveOptionsDialog::on_buttonBox_rejected);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(label);
    mainLayout->addWidget(threeMoveOptionComboBox);
    mainLayout->addWidget(buttonBox);
}

ThreeMoveOptionsDialog::~ThreeMoveOptionsDialog()
{
}

int ThreeMoveOptionsDialog::getSelectedThreeMoveOption() const
{
    return m_selectedThreeMoveOption;
}

void ThreeMoveOptionsDialog::on_buttonBox_accepted()
{
    m_selectedThreeMoveOption = threeMoveOptionComboBox->currentData().toInt();
    accept();
}

void ThreeMoveOptionsDialog::on_buttonBox_rejected()
{
    reject();
}
