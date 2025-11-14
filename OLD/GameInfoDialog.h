#ifndef GAMEINFODIALOG_H
#define GAMEINFODIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include "checkers_types.h" // For PDNgame struct

class GameInfoDialog : public QDialog
{
    Q_OBJECT

public:
    // Constructor takes necessary game info
    explicit GameInfoDialog(PDNgame& gameData, QWidget *parent = nullptr);
    ~GameInfoDialog();

    PDNgame getGameInfo() const; // New method to return updated game info

private:
    // UI Elements
    QLineEdit *eventEdit;
    QLineEdit *siteEdit;
    QLineEdit *dateEdit;
    QLineEdit *roundEdit;
    QLineEdit *whiteEdit;
    QLineEdit *blackEdit;
    QLineEdit *resultEdit;
    QLabel *fenLabel; // FEN is display-only
    QPushButton *okButton;
    QPushButton *cancelButton;

    // Layouts
    QFormLayout *formLayout;
    QVBoxLayout *mainLayout;

    PDNgame m_gameData; // Store a copy of the game data
};

#endif // GAMEINFODIALOG_H

