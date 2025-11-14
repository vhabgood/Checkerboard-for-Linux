/********************************************************************************
** Form generated from reading UI file 'EngineOptionsDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ENGINEOPTIONSDIALOG_H
#define UI_ENGINEOPTIONSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_EngineOptionsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *engineNameLabel;
    QPlainTextEdit *optionsPlainTextEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *EngineOptionsDialog)
    {
        if (EngineOptionsDialog->objectName().isEmpty())
            EngineOptionsDialog->setObjectName(QString::fromUtf8("EngineOptionsDialog"));
        EngineOptionsDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(EngineOptionsDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        engineNameLabel = new QLabel(EngineOptionsDialog);
        engineNameLabel->setObjectName(QString::fromUtf8("engineNameLabel"));

        verticalLayout->addWidget(engineNameLabel);

        optionsPlainTextEdit = new QPlainTextEdit(EngineOptionsDialog);
        optionsPlainTextEdit->setObjectName(QString::fromUtf8("optionsPlainTextEdit"));

        verticalLayout->addWidget(optionsPlainTextEdit);

        buttonBox = new QDialogButtonBox(EngineOptionsDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(EngineOptionsDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), EngineOptionsDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), EngineOptionsDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(EngineOptionsDialog);
    } // setupUi

    void retranslateUi(QDialog *EngineOptionsDialog)
    {
        EngineOptionsDialog->setWindowTitle(QCoreApplication::translate("EngineOptionsDialog", "Engine Options", nullptr));
        engineNameLabel->setText(QCoreApplication::translate("EngineOptionsDialog", "Engine Name:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class EngineOptionsDialog: public Ui_EngineOptionsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ENGINEOPTIONSDIALOG_H
