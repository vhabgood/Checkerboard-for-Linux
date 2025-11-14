#include "FindCRDialog.h"
#include "ui_FindCRDialog.h" // Include for ui_FindCRDialog.h

FindCRDialog::FindCRDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindCRDialog)
{
    ui->setupUi(this); // Setup any elements defined in the .ui file
    setWindowTitle(tr("Find CR"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *searchLabel = new QLabel(tr("Search String:"), this);
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText(tr("Enter search criteria here..."));

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchLineEdit);
    mainLayout->addLayout(searchLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FindCRDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FindCRDialog::on_buttonBox_rejected);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

FindCRDialog::~FindCRDialog()
{
    delete ui;
}

QString FindCRDialog::getSearchString() const
{
    return m_searchString;
}

void FindCRDialog::on_buttonBox_accepted()
{
    m_searchString = searchLineEdit->text();
    emit findCRRequested(m_searchString);
    accept();
}

void FindCRDialog::on_buttonBox_rejected()
{
    reject();
}
