#include "OptionsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QDebug>

OptionsDialog::OptionsDialog(const CBoptions& currentOptions, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Options"));
    setMinimumWidth(500);

    setupUi();

    // Populate tabs
    tabWidget->addTab(createDirectoriesTab(currentOptions), tr("Directories"));
    tabWidget->addTab(createTimingTab(currentOptions), tr("Timing"));
    tabWidget->addTab(createDisplayTab(currentOptions), tr("Display")); // Add Display tab
    tabWidget->addTab(createMiscTab(currentOptions), tr("Miscellaneous")); // Add Misc tab
    // Add other tabs here later...

    connect(buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &OptionsDialog::reject);
}

void OptionsDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);
}

QWidget* OptionsDialog::createDirectoriesTab(const CBoptions& options)
{
    // ... (Implementation remains the same) ...
    QWidget *tab = new QWidget;
    QFormLayout *layout = new QFormLayout(tab);

    userDirEdit = new QLineEdit(QString::fromUtf8(options.userdirectory));
    QPushButton *browseUserButton = new QPushButton(tr("Browse..."));
    connect(browseUserButton, &QPushButton::clicked, this, &OptionsDialog::browseUserDirectory);
    QHBoxLayout *userLayout = new QHBoxLayout;
    userLayout->addWidget(userDirEdit);
    userLayout->addWidget(browseUserButton);
    layout->addRow(tr("User Games Directory:"), userLayout);

    matchDirEdit = new QLineEdit(QString::fromUtf8(options.matchdirectory));
    QPushButton *browseMatchButton = new QPushButton(tr("Browse..."));
    connect(browseMatchButton, &QPushButton::clicked, this, &OptionsDialog::browseMatchDirectory);
    QHBoxLayout *matchLayout = new QHBoxLayout;
    matchLayout->addWidget(matchDirEdit);
    matchLayout->addWidget(browseMatchButton);
    layout->addRow(tr("Engine Match Directory:"), matchLayout);

    egtbDirEdit = new QLineEdit(QString::fromUtf8(options.EGTBdirectory));
    QPushButton *browseEGTBButton = new QPushButton(tr("Browse..."));
    connect(browseEGTBButton, &QPushButton::clicked, this, &OptionsDialog::browseEGTBDirectory);
    QHBoxLayout *egtbLayout = new QHBoxLayout;
    egtbLayout->addWidget(egtbDirEdit);
    egtbLayout->addWidget(browseEGTBButton);
    layout->addRow(tr("Endgame Databases Directory:"), egtbLayout);

    return tab;
}

QWidget* OptionsDialog::createTimingTab(const CBoptions& options)
{
    // ... (Implementation remains the same) ...
    QWidget *tab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(tab);
    QFormLayout *formLayout = new QFormLayout(); // Use Form layout for alignment

    // Level ComboBox
    levelComboBox = new QComboBox();
    // Add items with associated LEVEL_ enum data
    levelComboBox->addItem(tr("Instant"), LEVEL_INSTANT);
    levelComboBox->addItem(tr("0.1 Sec"), LEVEL_01S);
    levelComboBox->addItem(tr("0.2 Sec"), LEVEL_02S);
    levelComboBox->addItem(tr("0.5 Sec"), LEVEL_05S);
    levelComboBox->addItem(tr("1 Sec"), LEVEL_1S);
    levelComboBox->addItem(tr("2 Sec"), LEVEL_2S);
    levelComboBox->addItem(tr("5 Sec"), LEVEL_5S);
    levelComboBox->addItem(tr("10 Sec"), LEVEL_10S);
    levelComboBox->addItem(tr("15 Sec"), LEVEL_15S);
    levelComboBox->addItem(tr("30 Sec"), LEVEL_30S);
    levelComboBox->addItem(tr("1 Min"), LEVEL_1M);
    levelComboBox->addItem(tr("2 Min"), LEVEL_2M);
    levelComboBox->addItem(tr("5 Min"), LEVEL_5M);
    levelComboBox->addItem(tr("15 Min"), LEVEL_15M);
    levelComboBox->addItem(tr("30 Min"), LEVEL_30M);
    levelComboBox->addItem(tr("Infinite"), LEVEL_INFINITE);

    // Set current level
    int index = levelComboBox->findData(options.level);
    if (index != -1) {
        levelComboBox->setCurrentIndex(index);
    } else {
        levelComboBox->setCurrentIndex(levelComboBox->findData(LEVEL_1S)); // Default
    }
    formLayout->addRow(tr("Fixed Time Level:"), levelComboBox);

    // Exact Time CheckBox
    exactTimeCheckBox = new QCheckBox(tr("Use exact time per move"));
    exactTimeCheckBox->setChecked(options.exact_time);
    formLayout->addRow("", exactTimeCheckBox); // Add checkbox below level

    layout->addLayout(formLayout);
    layout->addSpacing(15);

    // Incremental Time Group
    incrementalGroup = new QGroupBox(tr("Incremental Time Control"));
    incrementalGroup->setCheckable(true);
    incrementalGroup->setChecked(options.use_incremental_time);
    connect(incrementalGroup, &QGroupBox::toggled, this, &OptionsDialog::updateIncrementalControls);

    QFormLayout *incrementalLayout = new QFormLayout(incrementalGroup);

    initialTimeSpinBox = new QDoubleSpinBox();
    initialTimeSpinBox->setRange(0.1, 9999.0);
    initialTimeSpinBox->setSuffix(tr(" sec"));
    initialTimeSpinBox->setValue(options.initial_time);
    incrementalLayout->addRow(tr("Initial Time:"), initialTimeSpinBox);

    incrementSpinBox = new QDoubleSpinBox();
    incrementSpinBox->setRange(0.0, 999.0);
    incrementSpinBox->setSuffix(tr(" sec"));
    incrementSpinBox->setValue(options.time_increment);
    incrementalLayout->addRow(tr("Increment per Move:"), incrementSpinBox);

    layout->addWidget(incrementalGroup);
    layout->addStretch(); // Push controls to the top

    // Initial state
    updateIncrementalControls(options.use_incremental_time);

    return tab;
}

QWidget* OptionsDialog::createDisplayTab(const CBoptions& options)
{
    QWidget *tab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(tab);

    invertCheckBox = new QCheckBox(tr("Invert Board"));
    invertCheckBox->setChecked(options.invert);
    layout->addWidget(invertCheckBox);

    numbersCheckBox = new QCheckBox(tr("Show Numbers"));
    numbersCheckBox->setChecked(options.numbers);
    layout->addWidget(numbersCheckBox);

    highlightCheckBox = new QCheckBox(tr("Highlight Last Move"));
    highlightCheckBox->setChecked(options.highlight);
    layout->addWidget(highlightCheckBox);

    mirrorCheckBox = new QCheckBox(tr("Mirror Board (for Italian/Spanish)"));
    mirrorCheckBox->setChecked(options.mirror);
    // Maybe disable this based on game type? For now, always show.
    layout->addWidget(mirrorCheckBox);

    // Add Piece Set selection later

    layout->addStretch(); // Push controls to the top
    return tab;
}

QWidget* OptionsDialog::createMiscTab(const CBoptions& options)
{
     QWidget *tab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(tab);

    soundCheckBox = new QCheckBox(tr("Enable Sound Effects"));
    soundCheckBox->setChecked(options.sound);
    layout->addWidget(soundCheckBox);

    userBookCheckBox = new QCheckBox(tr("Use User Opening Book"));
    userBookCheckBox->setChecked(options.userbook);
    layout->addWidget(userBookCheckBox);

    restrictOpeningCheckBox = new QCheckBox(tr("Restrict Opening Moves (3-Move)"));
    restrictOpeningCheckBox->setChecked(
        options.op_crossboard || options.op_mailplay || options.op_barred); // Approximation
    
    layout->addWidget(restrictOpeningCheckBox);

    enginePriorityCheckBox = new QCheckBox(tr("Run Engine at Lower Priority"));
    enginePriorityCheckBox->setChecked(options.priority); // Assuming 0=Normal, 1=Low
    layout->addWidget(enginePriorityCheckBox);


    // Add Language selection later
    // Add Engine Match options later

    layout->addStretch();
    return tab;
}


// --- Getters ---

QString OptionsDialog::getUserDirectory() const { return userDirEdit->text(); }
QString OptionsDialog::getMatchDirectory() const { return matchDirEdit->text(); }
QString OptionsDialog::getEGTBDirectory() const { return egtbDirEdit->text(); }

int OptionsDialog::getLevel() const { return levelComboBox->currentData().toInt(); }
bool OptionsDialog::getExactTime() const { return exactTimeCheckBox->isChecked(); }
bool OptionsDialog::getUseIncrementalTime() const { return incrementalGroup->isChecked(); }
double OptionsDialog::getInitialTime() const { return initialTimeSpinBox->value(); }
double OptionsDialog::getTimeIncrement() const { return incrementSpinBox->value(); }

bool OptionsDialog::getInvertBoard() const { return invertCheckBox->isChecked(); }
bool OptionsDialog::getShowNumbers() const { return numbersCheckBox->isChecked(); }
bool OptionsDialog::getHighlightMoves() const { return highlightCheckBox->isChecked(); }
bool OptionsDialog::getMirrorBoard() const { return mirrorCheckBox->isChecked(); }

bool OptionsDialog::getSoundEnabled() const { return soundCheckBox->isChecked(); }
bool OptionsDialog::getUseUserBook() const { return userBookCheckBox->isChecked(); }
bool OptionsDialog::getRestrictOpening() const { return restrictOpeningCheckBox->isChecked(); } // Simplified
bool OptionsDialog::getLowEnginePriority() const { return enginePriorityCheckBox->isChecked(); }


// --- Slots ---

// ... (Browse directory slots remain the same) ...
void OptionsDialog::browseUserDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select User Games Directory"), userDirEdit->text());
    if (!dir.isEmpty()) {
        userDirEdit->setText(dir);
    }
}

void OptionsDialog::browseMatchDirectory()
{
     QString dir = QFileDialog::getExistingDirectory(this, tr("Select Engine Match Directory"), matchDirEdit->text());
    if (!dir.isEmpty()) {
        matchDirEdit->setText(dir);
    }
}

void OptionsDialog::browseEGTBDirectory()
{
     QString dir = QFileDialog::getExistingDirectory(this, tr("Select Endgame Databases Directory"), egtbDirEdit->text());
    if (!dir.isEmpty()) {
        egtbDirEdit->setText(dir);
    }
}

void OptionsDialog::updateIncrementalControls(bool enabled)
{
    // GroupBox handles enabling/disabling children automatically
    // incrementalGroup->setEnabled(enabled); // This should work implicitly
}


