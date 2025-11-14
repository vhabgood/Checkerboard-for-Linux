#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "checkers_types.h" // For CBoptions struct
#include "CBconsts.h"       // For LEVEL_ constants

// Forward declarations
class QTabWidget;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QGroupBox;

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(const CBoptions& currentOptions, QWidget *parent = nullptr);

    // Getters for updated values
    // Directories
    QString getUserDirectory() const;
    QString getMatchDirectory() const;
    QString getEGTBDirectory() const;
    // Timing
    int getLevel() const;
    bool getExactTime() const;
    bool getUseIncrementalTime() const;
    double getInitialTime() const;
    double getTimeIncrement() const;
    // Display
    bool getInvertBoard() const;
    bool getShowNumbers() const;
    bool getHighlightMoves() const;
    bool getMirrorBoard() const;
    // Misc
    bool getSoundEnabled() const;
    bool getUseUserBook() const;
    bool getRestrictOpening() const; // Renamed from options3Move
    bool getLowEnginePriority() const; // Assuming 0 is Normal, 1 is Low


private slots:
    // Directories
    void browseUserDirectory();
    void browseMatchDirectory();
    void browseEGTBDirectory();
    // Timing
    void updateIncrementalControls(bool enabled);


private:
    void setupUi();
    QWidget* createDirectoriesTab(const CBoptions& options);
    QWidget* createTimingTab(const CBoptions& options);
    QWidget* createDisplayTab(const CBoptions& options); // New
    QWidget* createMiscTab(const CBoptions& options); // New

    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;

    // Directories Tab Widgets
    QLineEdit *userDirEdit;
    QLineEdit *matchDirEdit;
    QLineEdit *egtbDirEdit;

    // Timing Tab Widgets
    QComboBox *levelComboBox;
    QCheckBox *exactTimeCheckBox;
    QGroupBox *incrementalGroup;
    QDoubleSpinBox *initialTimeSpinBox;
    QDoubleSpinBox *incrementSpinBox;

    // Display Tab Widgets
    QCheckBox *invertCheckBox;
    QCheckBox *numbersCheckBox;
    QCheckBox *highlightCheckBox;
    QCheckBox *mirrorCheckBox;
    QComboBox *pieceSetComboBox;

    // Misc Tab Widgets
    QCheckBox *soundCheckBox;
    QCheckBox *userBookCheckBox;
    QCheckBox *restrictOpeningCheckBox;
    QCheckBox *enginePriorityCheckBox; // Renamed from optionsPriority

};

#endif // OPTIONSDIALOG_H


