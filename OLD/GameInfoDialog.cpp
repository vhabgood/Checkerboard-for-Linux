#include "GameInfoDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDebug>
#include <cstring> // For strncpy

GameInfoDialog::GameInfoDialog(PDNgame& gameData, QWidget *parent)
    : QDialog(parent),
      m_gameData(gameData) // Initialize m_gameData with the passed data
{
    setWindowTitle(tr("Game Information"));

    // Create QLineEdit widgets for editable fields
    eventEdit = new QLineEdit(QString::fromUtf8(m_gameData.event), this);
    siteEdit = new QLineEdit(QString::fromUtf8(m_gameData.site), this);
    dateEdit = new QLineEdit(QString::fromUtf8(m_gameData.date), this);
    roundEdit = new QLineEdit(QString::fromUtf8(m_gameData.round), this);
    whiteEdit = new QLineEdit(QString::fromUtf8(m_gameData.white), this);
    blackEdit = new QLineEdit(QString::fromUtf8(m_gameData.black), this);
    resultEdit = new QLineEdit(QString::fromUtf8(m_gameData.resultstring), this);
    fenLabel = new QLabel(QString::fromUtf8(m_gameData.FEN), this); // FEN remains display-only
    fenLabel->setWordWrap(true);

    // Create buttons
    okButton = new QPushButton(tr("OK"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);

    connect(okButton, &QPushButton::clicked, this, &GameInfoDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &GameInfoDialog::reject);

    // Layout
    formLayout = new QFormLayout();
    formLayout->addRow(tr("Event:"), eventEdit);
    formLayout->addRow(tr("Site:"), siteEdit);
    formLayout->addRow(tr("Date:"), dateEdit);
    formLayout->addRow(tr("Round:"), roundEdit);
    formLayout->addRow(tr("White:"), whiteEdit);
    formLayout->addRow(tr("Black:"), blackEdit);
    formLayout->addRow(tr("Result:"), resultEdit);
    formLayout->addRow(tr("FEN:"), fenLabel);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setMinimumWidth(400);
}

GameInfoDialog::~GameInfoDialog()
{
    // Qt handles child widget deletion with parent assignment
}

PDNgame GameInfoDialog::getGameInfo() const
{
    PDNgame updatedGame = m_gameData; // Start with a copy of the original data

    // Update fields from QLineEdit widgets
    strncpy(updatedGame.event, eventEdit->text().toUtf8().constData(), sizeof(updatedGame.event) - 1);
    updatedGame.event[sizeof(updatedGame.event) - 1] = '\0';

    strncpy(updatedGame.site, siteEdit->text().toUtf8().constData(), sizeof(updatedGame.site) - 1);
    updatedGame.site[sizeof(updatedGame.site) - 1] = '\0';

    strncpy(updatedGame.date, dateEdit->text().toUtf8().constData(), sizeof(updatedGame.date) - 1);
    updatedGame.date[sizeof(updatedGame.date) - 1] = '\0';

    strncpy(updatedGame.round, roundEdit->text().toUtf8().constData(), sizeof(updatedGame.round) - 1);
    updatedGame.round[sizeof(updatedGame.round) - 1] = '\0';

    strncpy(updatedGame.white, whiteEdit->text().toUtf8().constData(), sizeof(updatedGame.white) - 1);
    updatedGame.white[sizeof(updatedGame.white) - 1] = '\0';

    strncpy(updatedGame.black, blackEdit->text().toUtf8().constData(), sizeof(updatedGame.black) - 1);
    updatedGame.black[sizeof(updatedGame.black) - 1] = '\0';

    strncpy(updatedGame.resultstring, resultEdit->text().toUtf8().constData(), sizeof(updatedGame.resultstring) - 1);
    updatedGame.resultstring[sizeof(updatedGame.resultstring) - 1] = '\0';

    // FEN is display-only, so it's not updated here

    return updatedGame;
}