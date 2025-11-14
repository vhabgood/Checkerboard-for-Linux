#ifndef DIRECTORIESDIALOG_H
#define DIRECTORIESDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

class DirectoriesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DirectoriesDialog(const QString& currentUserDir, const QString& currentMatchDir, const QString& currentEGTBDir, QWidget *parent = nullptr);
    ~DirectoriesDialog();

    QString getUserDirectory() const;
    QString getMatchDirectory() const;
    QString getEGTBDirectory() const;

private slots:
    void browseUserDirectory();
    void browseMatchDirectory();
    void browseEGTBDirectory();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    QLineEdit *userDirectoryLineEdit;
    QLineEdit *matchDirectoryLineEdit;
    QLineEdit *egtbDirectoryLineEdit;

    QString m_userDirectory;
    QString m_matchDirectory;
    QString m_egtbDirectory;
};

#endif // DIRECTORIESDIALOG_H
