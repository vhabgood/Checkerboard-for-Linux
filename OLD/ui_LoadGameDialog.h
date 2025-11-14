/********************************************************************************
** Form generated from reading UI file 'LoadGameDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOADGAMEDIALOG_H
#define UI_LOADGAMEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_LoadGameDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLineEdit *fileNameLineEdit;
    QListView *fileListView;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *LoadGameDialog)
    {
        if (LoadGameDialog->objectName().isEmpty())
            LoadGameDialog->setObjectName(QString::fromUtf8("LoadGameDialog"));
        LoadGameDialog->resize(600, 400);
        verticalLayout = new QVBoxLayout(LoadGameDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(LoadGameDialog);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        fileNameLineEdit = new QLineEdit(LoadGameDialog);
        fileNameLineEdit->setObjectName(QString::fromUtf8("fileNameLineEdit"));
        fileNameLineEdit->setReadOnly(true);

        verticalLayout->addWidget(fileNameLineEdit);

        fileListView = new QListView(LoadGameDialog);
        fileListView->setObjectName(QString::fromUtf8("fileListView"));

        verticalLayout->addWidget(fileListView);

        buttonBox = new QDialogButtonBox(LoadGameDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(LoadGameDialog);

        QMetaObject::connectSlotsByName(LoadGameDialog);
    } // setupUi

    void retranslateUi(QDialog *LoadGameDialog)
    {
        LoadGameDialog->setWindowTitle(QCoreApplication::translate("LoadGameDialog", "Load Game", nullptr));
        label->setText(QCoreApplication::translate("LoadGameDialog", "Select a PDN file to load:", nullptr));
        fileNameLineEdit->setPlaceholderText(QCoreApplication::translate("LoadGameDialog", "No file selected", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoadGameDialog: public Ui_LoadGameDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOADGAMEDIALOG_H
