#include "EngineSelectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir> // For platform path separator
#include <QRadioButton>
#include <QGroupBox>

EngineSelectDialog::EngineSelectDialog(const QString& currentPrimary, const QString& currentSecondary, int currentEngine, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Select Engines"));

    // Primary Engine Section
    primaryLabel = new QLabel(tr("Primary Engine:"), this);
    primaryLineEdit = new QLineEdit(currentPrimary, this);
    primaryBrowseButton = new QPushButton(tr("Browse..."), this);
    connect(primaryBrowseButton, &QPushButton::clicked, this, &EngineSelectDialog::browsePrimary);

    QHBoxLayout *primaryLayout = new QHBoxLayout();
    primaryLayout->addWidget(primaryLineEdit);
    primaryLayout->addWidget(primaryBrowseButton);

    // Secondary Engine Section
    secondaryLabel = new QLabel(tr("Secondary Engine:"), this);
    secondaryLineEdit = new QLineEdit(currentSecondary, this);
    secondaryBrowseButton = new QPushButton(tr("Browse..."), this);
    connect(secondaryBrowseButton, &QPushButton::clicked, this, &EngineSelectDialog::browseSecondary);

    QHBoxLayout *secondaryLayout = new QHBoxLayout();
    secondaryLayout->addWidget(secondaryLineEdit);
    secondaryLayout->addWidget(secondaryBrowseButton);

    // Engine Selection Group
    QGroupBox *engineSelectionGroup = new QGroupBox(tr("Active Engine"), this);
    QVBoxLayout *engineSelectionLayout = new QVBoxLayout(engineSelectionGroup);

    m_noneEngineRadio = new QRadioButton(tr("None"), this);
    m_primaryEngineRadio = new QRadioButton(tr("Primary Engine"), this);
    m_secondaryEngineRadio = new QRadioButton(tr("Secondary Engine"), this);

    engineSelectionLayout->addWidget(m_noneEngineRadio);
    engineSelectionLayout->addWidget(m_primaryEngineRadio);
    engineSelectionLayout->addWidget(m_secondaryEngineRadio);

    // Set initial selection
    if (currentEngine == 1) {
        m_primaryEngineRadio->setChecked(true);
    } else if (currentEngine == 2) {
        m_secondaryEngineRadio->setChecked(true);
    } else {
        m_noneEngineRadio->setChecked(true);
    }

    // Buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &EngineSelectDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EngineSelectDialog::reject);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(primaryLabel);
    mainLayout->addLayout(primaryLayout);
    mainLayout->addWidget(secondaryLabel);
    mainLayout->addLayout(secondaryLayout);
    mainLayout->addWidget(engineSelectionGroup);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setMinimumWidth(450);
}

EngineSelectDialog::~EngineSelectDialog()
{
}

QString EngineSelectDialog::primaryEnginePath() const
{
    return primaryLineEdit->text();
}

QString EngineSelectDialog::secondaryEnginePath() const
{
    return secondaryLineEdit->text();
}

int EngineSelectDialog::selectedEngineIndex() const
{
    if (m_primaryEngineRadio->isChecked()) {
        return 1;
    } else if (m_secondaryEngineRadio->isChecked()) {
        return 2;
    } else {
        return 0; // None selected
    }
}

QString EngineSelectDialog::selectedEngineName() const
{
    if (m_primaryEngineRadio->isChecked()) {
        return tr("Primary Engine");
    } else if (m_secondaryEngineRadio->isChecked()) {
        return tr("Secondary Engine");
    } else {
        return tr("None");
    }
}

void EngineSelectDialog::browsePrimary()
{
    // Suggest starting in an 'engines' subdirectory if it exists
    QString startDir = QDir::currentPath() + QDir::separator() + "engines";
    if (!QDir(startDir).exists()) {
        startDir = QDir::currentPath();
    }

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select Primary Engine Library"), startDir,
        tr("Shared Libraries (*.so *.dll *.dylib);;All Files (*)"));

    if (!fileName.isEmpty()) {
        // Store just the filename if it's in the engines subdir, else full path?
        // For simplicity now, store what the user selected. loadengines prepends 'engines/' anyway.
        // Let's store just the filename for consistency with original behavior.
        QFileInfo fileInfo(fileName);
         primaryLineEdit->setText(fileInfo.fileName());
        // primaryLineEdit->setText(fileName); // Alternative: store full path
    }
}

void EngineSelectDialog::browseSecondary()
{
    QString startDir = QDir::currentPath() + QDir::separator() + "engines";
     if (!QDir(startDir).exists()) {
        startDir = QDir::currentPath();
    }

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select Secondary Engine Library"), startDir,
        tr("Shared Libraries (*.so *.dll *.dylib);;All Files (*)"));

    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        secondaryLineEdit->setText(fileInfo.fileName());
        // secondaryLineEdit->setText(fileName); // Alternative: store full path
    }
}