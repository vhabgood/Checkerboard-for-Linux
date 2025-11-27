#include "DirectoriesDialog.h"
#include <QFileDialog>

DirectoriesDialog::DirectoriesDialog(const QString& currentUserDir, const QString& currentMatchDir, const QString& currentEGTBDir, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Configure Directories"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // User Directory
    QHBoxLayout *userLayout = new QHBoxLayout();
    QLabel *userLabel = new QLabel(tr("User Directory:"), this);
    userDirectoryLineEdit = new QLineEdit(currentUserDir, this);
    QPushButton *userBrowseButton = new QPushButton(tr("Browse..."), this);
    connect(userBrowseButton, &QPushButton::clicked, this, &DirectoriesDialog::browseUserDirectory);
    userLayout->addWidget(userLabel);
    userLayout->addWidget(userDirectoryLineEdit);
    userLayout->addWidget(userBrowseButton);
    mainLayout->addLayout(userLayout);

    // Match Directory
    QHBoxLayout *matchLayout = new QHBoxLayout();
    QLabel *matchLabel = new QLabel(tr("Match Directory:"), this);
    matchDirectoryLineEdit = new QLineEdit(currentMatchDir, this);
    QPushButton *matchBrowseButton = new QPushButton(tr("Browse..."), this);
    connect(matchBrowseButton, &QPushButton::clicked, this, &DirectoriesDialog::browseMatchDirectory);
    matchLayout->addWidget(matchLabel);
    matchLayout->addWidget(matchDirectoryLineEdit);
    matchLayout->addWidget(matchBrowseButton);
    mainLayout->addLayout(matchLayout);

    // EGTB Directory
    QHBoxLayout *egtbLayout = new QHBoxLayout();
    QLabel *egtbLabel = new QLabel(tr("EGTB Directory:"), this);
    egtbDirectoryLineEdit = new QLineEdit(currentEGTBDir, this);
    QPushButton *egtbBrowseButton = new QPushButton(tr("Browse..."), this);
    connect(egtbBrowseButton, &QPushButton::clicked, this, &DirectoriesDialog::browseEGTBDirectory);
    egtbLayout->addWidget(egtbLabel);
    egtbLayout->addWidget(egtbDirectoryLineEdit);
    egtbLayout->addWidget(egtbBrowseButton);
    mainLayout->addLayout(egtbLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DirectoriesDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DirectoriesDialog::on_buttonBox_rejected);
    mainLayout->addWidget(buttonBox);
}

DirectoriesDialog::~DirectoriesDialog()
{
}

QString DirectoriesDialog::getUserDirectory() const
{
    return m_userDirectory;
}

QString DirectoriesDialog::getMatchDirectory() const
{
    return m_matchDirectory;
}

QString DirectoriesDialog::getEGTBDirectory() const
{
    return m_egtbDirectory;
}

void DirectoriesDialog::browseUserDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select User Directory"), userDirectoryLineEdit->text());
    if (!dir.isEmpty()) {
        userDirectoryLineEdit->setText(dir);
    }
}

void DirectoriesDialog::browseMatchDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Match Directory"), matchDirectoryLineEdit->text());
    if (!dir.isEmpty()) {
        matchDirectoryLineEdit->setText(dir);
    }
}

void DirectoriesDialog::browseEGTBDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select EGTB Directory"), egtbDirectoryLineEdit->text());
    if (!dir.isEmpty()) {
        egtbDirectoryLineEdit->setText(dir);
    }
}

void DirectoriesDialog::on_buttonBox_accepted()
{
    m_userDirectory = userDirectoryLineEdit->text();
    m_matchDirectory = matchDirectoryLineEdit->text();
    m_egtbDirectory = egtbDirectoryLineEdit->text();
    accept();
}

void DirectoriesDialog::on_buttonBox_rejected()
{
    reject();
}
