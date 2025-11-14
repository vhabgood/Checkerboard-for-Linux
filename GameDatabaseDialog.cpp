#include "GameDatabaseDialog.h"
#include "ui_GameDatabaseDialog.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QStandardPaths>
#include "GameManager.h" // To access PDN parsing functions

GameDatabaseDialog::GameDatabaseDialog(GameManager *gameManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GameDatabaseDialog),
    m_gameManager(gameManager) // Initialize GameManager pointer
{
    ui->setupUi(this); // Setup any elements defined in the .ui file

    setWindowTitle(tr("Game Database"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Search Line Edit
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText(tr("Search games..."));
    connect(searchLineEdit, &QLineEdit::textChanged, this, &GameDatabaseDialog::onSearchTextChanged);
    mainLayout->addWidget(searchLineEdit);

    // Game List Widget
    gameListWidget = new QListWidget(this);
    mainLayout->addWidget(gameListWidget);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    loadFileButton = new QPushButton(tr("Load PDN File"), this);
    connect(loadFileButton, &QPushButton::clicked, this, &GameDatabaseDialog::onLoadFileButtonClicked);
    buttonLayout->addWidget(loadFileButton);

    loadGameButton = new QPushButton(tr("Load Selected Game"), this);
    loadGameButton->setEnabled(false); // Disable until a game is selected
    connect(loadGameButton, &QPushButton::clicked, this, &GameDatabaseDialog::onLoadGameButtonClicked);
    buttonLayout->addWidget(loadGameButton);

    mainLayout->addLayout(buttonLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close); // Use rejected to close
    mainLayout->addWidget(buttonBox);

    // Connect list widget selection change to enable/disable loadGameButton
    connect(gameListWidget, &QListWidget::currentRowChanged, this, [this](int row){
        loadGameButton->setEnabled(row >= 0);
    });

    setLayout(mainLayout);
}

GameDatabaseDialog::~GameDatabaseDialog()
{
    delete ui;
}

void GameDatabaseDialog::onLoadFileButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open PDN File"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    tr("PDN Files (*.pdn);;All Files (*)"));
    if (!fileName.isEmpty()) {
        m_currentPdnFilePath = fileName;
        parsePdnFile(fileName);
        displayGames(m_loadedGames);
    }
}

void GameDatabaseDialog::onSearchTextChanged(const QString& text)
{
    QVector<PdnGameWrapper> filteredGames;
    for (const PdnGameWrapper& game : m_loadedGames) {
        QString gameTitle = QString("%1 vs. %2 (%3)").arg(game.game.white).arg(game.game.black).arg(game.game.date);
        if (gameTitle.contains(text, Qt::CaseInsensitive)) {
            filteredGames.append(game);
        }
    }
    displayGames(filteredGames);
}

void GameDatabaseDialog::onLoadGameButtonClicked()
{
    int currentRow = gameListWidget->currentRow();
    if (currentRow >= 0 && currentRow < m_loadedGames.size()) {
        emit gameSelected(m_loadedGames[currentRow]);
        accept(); // Close the dialog
    }
}

void GameDatabaseDialog::parsePdnFile(const QString& filename)
{
    m_loadedGames.clear();
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file: %1").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    QByteArray ba = fileContent.toUtf8();
    char* buffer = ba.data();
    char* current_pos = buffer;

    char game_str[10000]; // Buffer for a single game string
    while (m_gameManager->PDNparseGetnextgame(&current_pos, game_str, sizeof(game_str))) {
        PdnGameWrapper game;
        m_gameManager->parsePdnGameString(game_str, game);
        m_loadedGames.append(game);
    }
}

void GameDatabaseDialog::displayGames(const QVector<PdnGameWrapper>& gamesToDisplay)
{
    gameListWidget->clear();
    for (const PdnGameWrapper& game : gamesToDisplay) {
        QString gameTitle = QString("%1 vs. %2 (%3)").arg(game.game.white).arg(game.game.black).arg(game.game.date);
        gameListWidget->addItem(gameTitle);
    }
}
