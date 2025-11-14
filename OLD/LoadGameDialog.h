#ifndef LOADGAMEDIALOG_H
#define LOADGAMEDIALOG_H

#include <QDialog>
#include <QFileSystemModel>

namespace Ui { class LoadGameDialog; }

class LoadGameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadGameDialog(QWidget *parent = nullptr);
    ~LoadGameDialog();
    QString getSelectedFileName() const;

private slots:
    void on_fileListView_clicked(const QModelIndex &index);

private:
    Ui::LoadGameDialog *ui;
    QFileSystemModel *fileSystemModel;
    QString selectedFileName;
};

#endif // LOADGAMEDIALOG_H
