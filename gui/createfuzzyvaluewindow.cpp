#include "createfuzzyvaluewindow.h"
#include "ui_createfuzzyvaluewindow.h"

CreateFuzzyValueWindow::CreateFuzzyValueWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateFuzzyValueWindow)
{
    ui->setupUi(this);
}

CreateFuzzyValueWindow::~CreateFuzzyValueWindow()
{
    delete ui;
}
