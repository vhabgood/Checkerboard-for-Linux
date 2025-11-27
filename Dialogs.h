#ifndef DIALOGS_H
#define DIALOGS_H

#include <QDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QString>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVector>
#include <QRadioButton>
#include <QMessageBox> // <-- Added

namespace Ui {
class FindCRDialog;
class EngineOptionsDialog;
class FindPositionDialog;
}

class PriorityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PriorityDialog(int currentPriority, QWidget *parent = nullptr);
    ~PriorityDialog();

    int getSelectedPriority() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    QComboBox *priorityComboBox;
    int m_selectedPriority;
};


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

class ThreeMoveOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ThreeMoveOptionsDialog(int currentThreeMoveOption, QWidget *parent = nullptr);
    ~ThreeMoveOptionsDialog();

    int getSelectedThreeMoveOption() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    QComboBox *threeMoveOptionComboBox;
    int m_selectedThreeMoveOption;
};

class FindCRDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindCRDialog(QWidget *parent = nullptr);
    ~FindCRDialog();

    QString getSearchString() const;

signals:
    void findCRRequested(const QString& searchString);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::FindCRDialog *ui;
    QLineEdit *searchLineEdit;
    QString m_searchString;
};

class EngineOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EngineOptionsDialog(const QString& engineName, const QString& currentOptions, QWidget *parent = nullptr);
    ~EngineOptionsDialog();

    QString getNewOptions() const;
    QString engineName() const;
    QString engineOptions() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::EngineOptionsDialog *ui;
    QString m_engineName;
    QString m_currentOptions;
    QString m_newOptions;
};

class FindPositionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindPositionDialog(QWidget *parent = nullptr);
    ~FindPositionDialog();

    QString getFenString() const;

signals:
    void findPositionRequested(const QString& fen);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::FindPositionDialog *ui;
    QLineEdit *fenLineEdit;
    QString m_fenString;
};

class EngineSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EngineSelectDialog(const QString& currentPrimary, const QString& currentSecondary, int currentEngine, QWidget *parent = nullptr);
    ~EngineSelectDialog();

    QString primaryEnginePath() const;
    QString secondaryEnginePath() const;
    int selectedEngineIndex() const; // Renamed from getSelectedEngine()
    QString selectedEngineName() const;

private slots:
    void browsePrimary();
    void browseSecondary();

private:
    // UI Elements
    QLabel *primaryLabel;
    QLineEdit *primaryLineEdit;
    QPushButton *primaryBrowseButton;

    QLabel *secondaryLabel;
    QLineEdit *secondaryLineEdit;
    QPushButton *secondaryBrowseButton;

    QRadioButton *m_noneEngineRadio;
    QRadioButton *m_primaryEngineRadio;
    QRadioButton *m_secondaryEngineRadio;

    QDialogButtonBox *buttonBox;

    // Layouts (will be created in .cpp)
};

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


#endif // DIALOGS_H