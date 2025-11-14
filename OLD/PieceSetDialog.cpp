#include "PieceSetDialog.h"
#include <QDir>
#include <QDebug>
#include <QMessageBox>

PieceSetDialog::PieceSetDialog(const QString& currentPieceSet, QWidget *parent)
    : QDialog(parent),
      m_selectedPieceSet(currentPieceSet)
{
    setWindowTitle(tr("Select Piece Set"));

    m_pieceSetListWidget = new QListWidget(this);
    loadPieceSets();

    // Select the current piece set in the list
    if (!m_selectedPieceSet.isEmpty()) {
        QList<QListWidgetItem*> items = m_pieceSetListWidget->findItems(m_selectedPieceSet, Qt::MatchExactly);
        if (!items.isEmpty()) {
            m_pieceSetListWidget->setCurrentItem(items.first());
        }
    }

    QPushButton *okButton = new QPushButton(tr("OK"), this);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);

    connect(okButton, &QPushButton::clicked, this, &PieceSetDialog::acceptSelection);
    connect(cancelButton, &QPushButton::clicked, this, &PieceSetDialog::rejectSelection);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_pieceSetListWidget);
    mainLayout->addLayout(buttonLayout);
}

PieceSetDialog::~PieceSetDialog()
{
}

void PieceSetDialog::loadPieceSets()
{
    // Assuming piece sets are subdirectories within the "bmp" directory
    QDir bmpDir(":/bmp"); // Access resources via Qt resource system
    if (!bmpDir.exists()) {
        qWarning() << "Resource directory ':/bmp' does not exist.";
        QMessageBox::warning(this, tr("Error"), tr("Piece set directory not found."));
        return;
    }

    m_pieceSetListWidget->clear();
    m_availablePieceSets.clear();

    // Get a list of all subdirectories (each representing a piece set)
    QStringList entries = bmpDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (entries.isEmpty()) {
        qWarning() << "No piece sets found in ':/bmp'.";
        QMessageBox::information(this, tr("Info"), tr("No piece sets found."));
        return;
    }

    foreach (const QString &dirName, entries) {
        m_pieceSetListWidget->addItem(dirName);
        m_availablePieceSets.append(dirName);
    }

    if (!m_availablePieceSets.isEmpty() && m_pieceSetListWidget->currentItem() == nullptr) {
        m_pieceSetListWidget->setCurrentRow(0); // Select the first item if nothing is selected
    }
}

void PieceSetDialog::acceptSelection()
{
    QListWidgetItem *selectedItem = m_pieceSetListWidget->currentItem();
    if (selectedItem) {
        m_selectedPieceSet = selectedItem->text();
        QDialog::accept();
    } else {
        QMessageBox::warning(this, tr("Selection Error"), tr("Please select a piece set."));
    }
}

void PieceSetDialog::rejectSelection()
{
    QDialog::reject();
}