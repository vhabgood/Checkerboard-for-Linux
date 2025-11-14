/********************************************************************************
** Form generated from reading UI file 'EngineCommandDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ENGINECOMMANDDIALOG_H
#define UI_ENGINECOMMANDDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_EngineCommandDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLineEdit *commandLineEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *EngineCommandDialog)
    {
        if (EngineCommandDialog->objectName().isEmpty())
            EngineCommandDialog->setObjectName(QString::fromUtf8("EngineCommandDialog"));
        EngineCommandDialog->resize(400, 150);
        verticalLayout = new QVBoxLayout(EngineCommandDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(EngineCommandDialog);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        commandLineEdit = new QLineEdit(EngineCommandDialog);
        commandLineEdit->setObjectName(QString::fromUtf8("commandLineEdit"));

        verticalLayout->addWidget(commandLineEdit);

        buttonBox = new QDialogButtonBox(EngineCommandDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(EngineCommandDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), EngineCommandDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), EngineCommandDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(EngineCommandDialog);
    } // setupUi

    void retranslateUi(QDialog *EngineCommandDialog)
    {
        EngineCommandDialog->setWindowTitle(QCoreApplication::translate("EngineCommandDialog", "Engine Command", nullptr));
        label->setText(QCoreApplication::translate("EngineCommandDialog", "Enter Engine Command:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class EngineCommandDialog: public Ui_EngineCommandDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ENGINECOMMANDDIALOG_H
