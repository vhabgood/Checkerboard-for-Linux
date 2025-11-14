#ifndef PIECESETDIALOG_H
#define PIECESETDIALOG_H

#include <QDialog>
#include <QString>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>

class PieceSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PieceSetDialog(const QString& currentPieceSet, QWidget *parent = nullptr);
    ~PieceSetDialog();

    QString getSelectedPieceSet() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    QListWidget *pieceSetListWidget;
    QString m_selectedPieceSet;
};

#endif // PIECESETDIALOG_H
