/********************************************************************************
** Form generated from reading UI file 'FindPositionDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FINDPOSITIONDIALOG_H
#define UI_FINDPOSITIONDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_FindPositionDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *FindPositionDialog)
    {
        if (FindPositionDialog->objectName().isEmpty())
            FindPositionDialog->setObjectName(QString::fromUtf8("FindPositionDialog"));
        FindPositionDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(FindPositionDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(FindPositionDialog);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        buttonBox = new QDialogButtonBox(FindPositionDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(FindPositionDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), FindPositionDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), FindPositionDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(FindPositionDialog);
    } // setupUi

    void retranslateUi(QDialog *FindPositionDialog)
    {
        FindPositionDialog->setWindowTitle(QCoreApplication::translate("FindPositionDialog", "Find Position", nullptr));
        label->setText(QCoreApplication::translate("FindPositionDialog", "Find Position functionality will be implemented here.", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FindPositionDialog: public Ui_FindPositionDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FINDPOSITIONDIALOG_H
