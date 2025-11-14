#include "UserBookDialog.h"
#include <QFileDialog>
#include <QStandardPaths>

UserBookDialog::UserBookDialog(const QString& currentUserBookPath, bool readOnly, QWidget *parent)
    : QDialog(parent),
      m_currentUserBookPath(currentUserBookPath)
{
    setWindowTitle(tr("User Book Management"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    currentBookLabel = new QLabel(tr("Current User Book: %1").arg(m_currentUserBookPath.isEmpty() ? "None" : m_currentUserBookPath), this);
    mainLayout->addWidget(currentBookLabel);

    QHBoxLayout *fileButtonsLayout = new QHBoxLayout();
    loadButton = new QPushButton(tr("Load Book"), this);
    saveButton = new QPushButton(tr("Save Book"), this);
    fileButtonsLayout->addWidget(loadButton);
    fileButtonsLayout->addWidget(saveButton);
    mainLayout->addLayout(fileButtonsLayout);

    QHBoxLayout *entryButtonsLayout = new QHBoxLayout();
    addButton = new QPushButton(tr("Add Entry"), this);
    deleteButton = new QPushButton(tr("Delete Entry"), this);
    entryButtonsLayout->addWidget(addButton);
    entryButtonsLayout->addWidget(deleteButton);
    mainLayout->addLayout(entryButtonsLayout);

    if (readOnly) {
        loadButton->setEnabled(false);
        saveButton->setEnabled(false);
        addButton->setEnabled(false);
        deleteButton->setEnabled(false);
    }

    QHBoxLayout *navButtonsLayout = new QHBoxLayout();
    previousButton = new QPushButton(tr("Previous Entry"), this);
    nextButton = new QPushButton(tr("Next Entry"), this);
    resetButton = new QPushButton(tr("Reset Navigation"), this);
    navButtonsLayout->addWidget(previousButton);
    navButtonsLayout->addWidget(nextButton);
    navButtonsLayout->addWidget(resetButton);
    mainLayout->addLayout(navButtonsLayout);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    // Connections
    connect(loadButton, &QPushButton::clicked, this, &UserBookDialog::onLoadButtonClicked);
    connect(saveButton, &QPushButton::clicked, this, &UserBookDialog::onSaveButtonClicked);
    connect(addButton, &QPushButton::clicked, this, &UserBookDialog::onAddButtonClicked);
    connect(deleteButton, &QPushButton::clicked, this, &UserBookDialog::onDeleteButtonClicked);
    connect(nextButton, &QPushButton::clicked, this, &UserBookDialog::onNextButtonClicked);
    connect(previousButton, &QPushButton::clicked, this, &UserBookDialog::onPreviousButtonClicked);
    connect(resetButton, &QPushButton::clicked, this, &UserBookDialog::onResetButtonClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &UserBookDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &UserBookDialog::on_buttonBox_rejected);
}

UserBookDialog::~UserBookDialog()
{
}

QString UserBookDialog::getSelectedUserBookPath() const
{
    return m_selectedUserBookPath;
}

void UserBookDialog::onLoadButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load User Book"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    tr("User Book Files (*.ubk);;All Files (*)")).trimmed();
    if (!fileName.isEmpty()) {
        m_selectedUserBookPath = fileName;
        currentBookLabel->setText(tr("Current User Book: %1").arg(m_selectedUserBookPath));
        emit loadUserBookRequested(fileName);
    }
}

void UserBookDialog::onSaveButtonClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save User Book"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/userbook.ubk",
                                                    tr("User Book Files (*.ubk);;All Files (*)")).trimmed();
    if (!fileName.isEmpty()) {
        m_selectedUserBookPath = fileName; // Update selected path for consistency
        currentBookLabel->setText(tr("Current User Book: %1").arg(m_selectedUserBookPath));
        emit saveUserBookRequested(fileName);
    }
}

void UserBookDialog::onAddButtonClicked()
{
    emit addMoveToUserBookRequested();
}

void UserBookDialog::onDeleteButtonClicked()
{
    emit deleteCurrentEntryRequested();
}

void UserBookDialog::onNextButtonClicked()
{
    emit navigateToNextEntryRequested();
}

void UserBookDialog::onPreviousButtonClicked()
{
    emit navigateToPreviousEntryRequested();
}

void UserBookDialog::onResetButtonClicked()
{
    emit resetNavigationRequested();
}

void UserBookDialog::on_buttonBox_accepted()
{
    accept();
}

void UserBookDialog::on_buttonBox_rejected()
{
    reject();
}
