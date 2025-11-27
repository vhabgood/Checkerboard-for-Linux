#include "Dialogs.h"
#include <QDir>
#include <QMessageBox>
#include "ui_FindCRDialog.h"
#include "ui_EngineOptionsDialog.h"
#include "ui_FindPositionDialog.h"
#include <QFileDialog>
#include <QGroupBox>
#include <QFileInfo>
#include <QStandardPaths>

// --- PriorityDialog Implementation ---

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

// --- PieceSetDialog Implementation ---

PieceSetDialog::PieceSetDialog(const QString& currentPieceSet, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Select Piece Set"));

    pieceSetListWidget = new QListWidget(this);
    QDir bmpDir("bmp"); // Assuming bmp folder is in the ResourceFiles directory
    QStringList pieceSetDirs = bmpDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& dir : pieceSetDirs) {
        pieceSetListWidget->addItem(dir);
    }

    // Select the current piece set
    QList<QListWidgetItem *> items = pieceSetListWidget->findItems(currentPieceSet, Qt::MatchExactly);
    if (!items.isEmpty()) {
        pieceSetListWidget->setCurrentItem(items.first());
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PieceSetDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PieceSetDialog::on_buttonBox_rejected);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(pieceSetListWidget);
    mainLayout->addWidget(buttonBox);
}

PieceSetDialog::~PieceSetDialog()
{
}

QString PieceSetDialog::getSelectedPieceSet() const
{
    return m_selectedPieceSet;
}

void PieceSetDialog::on_buttonBox_accepted()
{
    if (pieceSetListWidget->currentItem()) {
        m_selectedPieceSet = pieceSetListWidget->currentItem()->text();
        accept();
    } else {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a piece set."));
    }
}

void PieceSetDialog::on_buttonBox_rejected()
{
    reject();
}

// --- ThreeMoveOptionsDialog Implementation ---

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

// --- FindCRDialog Implementation ---
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

// --- EngineOptionsDialog Implementation ---
EngineOptionsDialog::EngineOptionsDialog(const QString& engineName, const QString& currentOptions, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EngineOptionsDialog),
    m_engineName(engineName),
    m_currentOptions(currentOptions)
{
    ui->setupUi(this);
    setWindowTitle(tr("Engine Options for ") + m_engineName);
    ui->engineNameLabel->setText(m_engineName);
    ui->optionsPlainTextEdit->setPlainText(m_currentOptions);
}

EngineOptionsDialog::~EngineOptionsDialog()
{
    delete ui;
}

QString EngineOptionsDialog::getNewOptions() const
{
    return m_newOptions;
}

QString EngineOptionsDialog::engineName() const
{
    return m_engineName;
}

QString EngineOptionsDialog::engineOptions() const
{
    return m_newOptions; // Assuming m_newOptions holds the updated options
}

void EngineOptionsDialog::on_buttonBox_accepted()
{
    m_newOptions = ui->optionsPlainTextEdit->toPlainText();
    accept();
}

void EngineOptionsDialog::on_buttonBox_rejected()
{
    reject();
}

// --- FindPositionDialog Implementation ---
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

// --- EngineSelectDialog Implementation ---
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

// --- DirectoriesDialog Implementation ---
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

// --- UserBookDialog Implementation ---
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