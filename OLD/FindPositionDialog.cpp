#include "FindPositionDialog.h"
#include "ui_FindPositionDialog.h" // Include for ui_FindPositionDialog.h

FindPositionDialog::FindPositionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindPositionDialog)
{
    ui->setupUi(this); // Setup any elements defined in the .ui file
    setWindowTitle(tr("Find Position"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *fenLabel = new QLabel(tr("FEN String:"), this);
    fenLineEdit = new QLineEdit(this);
    fenLineEdit->setPlaceholderText(tr("Enter FEN string here..."));

    QHBoxLayout *fenLayout = new QHBoxLayout();
    fenLayout->addWidget(fenLabel);
    fenLayout->addWidget(fenLineEdit);
    mainLayout->addLayout(fenLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FindPositionDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FindPositionDialog::on_buttonBox_rejected);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

FindPositionDialog::~FindPositionDialog()
{
    delete ui;
}

QString FindPositionDialog::getFenString() const
{
    return m_fenString;
}

void FindPositionDialog::on_buttonBox_accepted()
{
    m_fenString = fenLineEdit->text();
    emit findPositionRequested(m_fenString);
    accept();
}

void FindPositionDialog::on_buttonBox_rejected()
{
    reject();
}
