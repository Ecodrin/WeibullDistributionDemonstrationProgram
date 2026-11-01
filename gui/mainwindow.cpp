#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_CreateFuzzyValueButton_clicked()
{
    CreateFuzzyValueWindow* create_fuzzy_window = new CreateFuzzyValueWindow(this);
    create_fuzzy_window->setAttribute(Qt::WA_DeleteOnClose);
    create_fuzzy_window->exec();
}

