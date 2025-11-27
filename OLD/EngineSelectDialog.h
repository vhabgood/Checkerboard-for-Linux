#ifndef ENGINESELECTDIALOG_H
#define ENGINESELECTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QString>
#include <QRadioButton>

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

#endif // ENGINESELECTDIALOG_H

