#include "PieceSetDialog.h"
#include <QDir>
#include <QMessageBox>

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
