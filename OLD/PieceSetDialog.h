#pragma once

#include <QDialog>
#include <QStringList>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class PieceSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PieceSetDialog(const QString& currentPieceSet, QWidget *parent = nullptr);
    ~PieceSetDialog();

    QString getSelectedPieceSet() const { return m_selectedPieceSet; }

private slots:
    void acceptSelection();
    void rejectSelection();

private:
    void loadPieceSets();

    QString m_selectedPieceSet;
    QListWidget *m_pieceSetListWidget;
    QStringList m_availablePieceSets;
};
