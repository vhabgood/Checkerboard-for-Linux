#ifndef USERBOOKDIALOG_H
#define USERBOOKDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>

#include "checkers_types.h" // For userbookentry

class UserBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserBookDialog(const QString& currentUserBookPath, bool readOnly = false, QWidget *parent = nullptr);
    ~UserBookDialog();

    QString getSelectedUserBookPath() const;

signals:
    void loadUserBookRequested(const QString& filename);
    void saveUserBookRequested(const QString& filename);
    void addMoveToUserBookRequested();
    void deleteCurrentEntryRequested();
    void navigateToNextEntryRequested();
    void navigateToPreviousEntryRequested();
    void resetNavigationRequested();

private slots:
    void onLoadButtonClicked();
    void onSaveButtonClicked();
    void onAddButtonClicked();
    void onDeleteButtonClicked();
    void onNextButtonClicked();
    void onPreviousButtonClicked();
    void onResetButtonClicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    QString m_currentUserBookPath;
    QString m_selectedUserBookPath;

    // UI Elements
    QLabel *currentBookLabel;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *nextButton;
    QPushButton *previousButton;
    QPushButton *resetButton;
    QDialogButtonBox *buttonBox;
};

#endif // USERBOOKDIALOG_H
