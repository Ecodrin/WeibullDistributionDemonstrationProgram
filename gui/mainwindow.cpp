#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->graphicsView->setBackgroundBrush(QBrush(QColor("#09090b")));
    ui->CountGenerateFuzzyValuesspinBox->setButtonSymbols(
        QAbstractSpinBox::UpDownArrows
        );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_CreateFuzzyValueButton_clicked()
{
    fuzzy_logic::fuzzy_value new_value;
    fuzzy_values.push_back(new_value);
    CreateFuzzyValueWindow* create_fuzzy_window = new CreateFuzzyValueWindow(fuzzy_values[fuzzy_values.size()-1], this, false);

    connect(create_fuzzy_window, &CreateFuzzyValueWindow::update_fuzzy_values_list_signal, this, &MainWindow::update_fuzzy_values_list);
    create_fuzzy_window->setAttribute(Qt::WA_DeleteOnClose);
    create_fuzzy_window->exec();


}

void MainWindow::update_fuzzy_values_list() {
    ui->FuzzyValueslistWidget->clear();
    for(size_t i = 0; i < fuzzy_values.size();++i) {
        ui->FuzzyValueslistWidget->addItem(QString::fromStdString(std::to_string(i)));
    }
}

void MainWindow::on_GenerateFuzzyValuesButton_clicked()
{
    size_t count_generate = ui->CountGenerateFuzzyValuesspinBox->value();
    for(size_t i = 0; i < count_generate; ++i) {
        fuzzy_logic::fuzzy_value v = fuzzy_logic::generate_random_fuzzy_value();
        fuzzy_values.push_back(v);
    }
    update_fuzzy_values_list();
}


void MainWindow::on_FuzzyValueslistWidget_itemDoubleClicked(QListWidgetItem *item)
{
    size_t index = item->text().toULong();
    // qDebug() << index << " " << fuzzy_values.size() << " " << fuzzy_values[index].get_alphas().size() << " " << fuzzy_values[index].get_alphas_intervals().size();
    CreateFuzzyValueWindow* create_fuzzy_window = new CreateFuzzyValueWindow(fuzzy_values[index], this, true);

    connect(create_fuzzy_window, &CreateFuzzyValueWindow::update_fuzzy_values_list_signal, this, &MainWindow::update_fuzzy_values_list);
    create_fuzzy_window->setAttribute(Qt::WA_DeleteOnClose);
    create_fuzzy_window->exec();
}

