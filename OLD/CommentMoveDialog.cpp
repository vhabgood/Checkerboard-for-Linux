#include "CommentMoveDialog.h"
#include "ui_CommentMoveDialog.h"

CommentMoveDialog::CommentMoveDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommentMoveDialog)
{
    ui->setupUi(this);
}

CommentMoveDialog::~CommentMoveDialog()
{
    delete ui;
}

QString CommentMoveDialog::getCommentText() const
{
    return ui->commentTextEdit->toPlainText();
}

void CommentMoveDialog::setCommentText(const QString &text)
{
    ui->commentTextEdit->setText(text);
}
