#include "LoadGameDialog.h"
#include "ui_LoadGameDialog.h"
#include <QDir>
#include <QDebug>

LoadGameDialog::LoadGameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadGameDialog)
{
    ui->setupUi(this);

    selectedFileName.clear();
    ui->fileNameLineEdit->clear();

    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    fileSystemModel->setNameFilters(QStringList() << "*.pdn");
    fileSystemModel->setNameFilterDisables(false);

    // Set the root path to the current working directory or a specific game directory
    // For now, let's assume PDN files are in the current directory or a 'games' subdirectory
    QString currentPath = QDir::currentPath();
    QString gamesPath = currentPath + "/games"; // Example: look in a 'games' subdirectory
    if (QDir(gamesPath).exists()) {
        fileSystemModel->setRootPath(gamesPath);
        ui->fileListView->setRootIndex(fileSystemModel->index(gamesPath));
    } else {
        fileSystemModel->setRootPath(currentPath);
        ui->fileListView->setRootIndex(fileSystemModel->index(currentPath));
    }

    ui->fileListView->setModel(fileSystemModel);
    ui->fileListView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->fileListView->setSelectionRectVisible(true);

    connect(ui->fileListView, &QListView::clicked, this, &LoadGameDialog::on_fileListView_clicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LoadGameDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &LoadGameDialog::reject);
}

LoadGameDialog::~LoadGameDialog()
{
    delete ui;
}

QString LoadGameDialog::getSelectedFileName() const
{
    return selectedFileName;
}

void LoadGameDialog::on_fileListView_clicked(const QModelIndex &index)
{
    if (index.isValid()) {
        selectedFileName = fileSystemModel->filePath(index);
        ui->fileNameLineEdit->setText(selectedFileName);
    }
}
