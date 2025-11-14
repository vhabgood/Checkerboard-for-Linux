/********************************************************************************
** Form generated from reading UI file 'FindCRDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FINDCRDIALOG_H
#define UI_FINDCRDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_FindCRDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *FindCRDialog)
    {
        if (FindCRDialog->objectName().isEmpty())
            FindCRDialog->setObjectName(QString::fromUtf8("FindCRDialog"));
        FindCRDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(FindCRDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(FindCRDialog);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        buttonBox = new QDialogButtonBox(FindCRDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(FindCRDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), FindCRDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), FindCRDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(FindCRDialog);
    } // setupUi

    void retranslateUi(QDialog *FindCRDialog)
    {
        FindCRDialog->setWindowTitle(QCoreApplication::translate("FindCRDialog", "Find CR", nullptr));
        label->setText(QCoreApplication::translate("FindCRDialog", "Find CR functionality will be implemented here.", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FindCRDialog: public Ui_FindCRDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FINDCRDIALOG_H
