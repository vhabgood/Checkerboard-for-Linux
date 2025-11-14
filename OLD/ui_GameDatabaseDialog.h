/********************************************************************************
** Form generated from reading UI file 'GameDatabaseDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GAMEDATABASEDIALOG_H
#define UI_GAMEDATABASEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_GameDatabaseDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *GameDatabaseDialog)
    {
        if (GameDatabaseDialog->objectName().isEmpty())
            GameDatabaseDialog->setObjectName(QString::fromUtf8("GameDatabaseDialog"));
        GameDatabaseDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(GameDatabaseDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(GameDatabaseDialog);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        buttonBox = new QDialogButtonBox(GameDatabaseDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(GameDatabaseDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), GameDatabaseDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), GameDatabaseDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(GameDatabaseDialog);
    } // setupUi

    void retranslateUi(QDialog *GameDatabaseDialog)
    {
        GameDatabaseDialog->setWindowTitle(QCoreApplication::translate("GameDatabaseDialog", "Game Database", nullptr));
        label->setText(QCoreApplication::translate("GameDatabaseDialog", "Game Database functionality will be implemented here.", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GameDatabaseDialog: public Ui_GameDatabaseDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GAMEDATABASEDIALOG_H
