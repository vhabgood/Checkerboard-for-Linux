#ifndef COMMENTMOVEDIALOG_H
#define COMMENTMOVEDIALOG_H

#include <QDialog>

namespace Ui {
class CommentMoveDialog;
}

class CommentMoveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommentMoveDialog(QWidget *parent = nullptr);
    ~CommentMoveDialog();

    QString getCommentText() const;
    void setCommentText(const QString &text);

private:
    Ui::CommentMoveDialog *ui;
};

#endif // COMMENTMOVEDIALOG_H
