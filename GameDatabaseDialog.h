#ifndef GAMEDATABASEDIALOG_H
#define GAMEDATABASEDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVector>
#include "GameManager.h" // Include GameManager to access its PDN parsing functions
#include "checkers_types.h" // For PdnGameWrapper

namespace Ui {
class GameDatabaseDialog;
}

class GameManager; // Forward declaration

class GameDatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameDatabaseDialog(GameManager *gameManager, QWidget *parent = nullptr);
    ~GameDatabaseDialog();

signals:
    void loadPdnGameRequested(const QString& filename);
    void gameSelected(const PdnGameWrapper& game);

private slots:
    void onLoadFileButtonClicked();
    void onSearchTextChanged(const QString& text);
    void onLoadGameButtonClicked();

private:
    Ui::GameDatabaseDialog *ui;
    QListWidget *gameListWidget;
    QLineEdit *searchLineEdit;
    QPushButton *loadFileButton;
    QPushButton *loadGameButton;
    QVector<PdnGameWrapper> m_loadedGames; // Store loaded games
    QString m_currentPdnFilePath; // Store the path of the currently loaded PDN file

    void parsePdnFile(const QString& filename);
    void displayGames(const QVector<PdnGameWrapper>& gamesToDisplay);

    GameManager *m_gameManager; // Pointer to GameManager instance
};

#endif // GAMEDATABASEDIALOG_H
