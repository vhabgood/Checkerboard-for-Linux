/********************************************************************************
** Form generated from reading UI file 'CommentMoveDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMMENTMOVEDIALOG_H
#define UI_COMMENTMOVEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CommentMoveDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QTextEdit *commentTextEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *CommentMoveDialog)
    {
        if (CommentMoveDialog->objectName().isEmpty())
            CommentMoveDialog->setObjectName(QString::fromUtf8("CommentMoveDialog"));
        CommentMoveDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(CommentMoveDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(CommentMoveDialog);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        commentTextEdit = new QTextEdit(CommentMoveDialog);
        commentTextEdit->setObjectName(QString::fromUtf8("commentTextEdit"));

        verticalLayout->addWidget(commentTextEdit);

        buttonBox = new QDialogButtonBox(CommentMoveDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(CommentMoveDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), CommentMoveDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), CommentMoveDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(CommentMoveDialog);
    } // setupUi

    void retranslateUi(QDialog *CommentMoveDialog)
    {
        CommentMoveDialog->setWindowTitle(QCoreApplication::translate("CommentMoveDialog", "Comment Move", nullptr));
        label->setText(QCoreApplication::translate("CommentMoveDialog", "Enter your comment for the move:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CommentMoveDialog: public Ui_CommentMoveDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMMENTMOVEDIALOG_H
