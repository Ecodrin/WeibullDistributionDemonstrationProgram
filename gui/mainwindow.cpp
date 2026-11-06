#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->ClasterQuickWidget->setClearColor(QColor("#18181b"));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_CreateFuzzyValueButton_clicked()
{
    CreateFuzzyValueWindow* create_fuzzy_window = new CreateFuzzyValueWindow(fuzzy_logic::fuzzy_value{}, this, false);

    connect(create_fuzzy_window, &CreateFuzzyValueWindow::add_fuzzy_value_to_list_signal, this, &MainWindow::add_fuzzy_value_to_list);
    create_fuzzy_window->setAttribute(Qt::WA_DeleteOnClose);
    create_fuzzy_window->exec();
}

void MainWindow::update_fuzzy_values_list()
{
    ui->FuzzyValueslistWidget->clear();
    for(size_t i = 0; i < fuzzy_values.size();++i) {
        ui->FuzzyValueslistWidget->addItem(QString::fromStdString(std::to_string(i)));
    }
    update_values_in_base_operations();
    update_count_clasters_clasess();
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

void MainWindow::add_fuzzy_value_to_list(fuzzy_logic::fuzzy_value val)
{
    if(val.get_alphas_intervals().size() == val.get_alphas().size() && val.get_alphas().size() > 0) {
        fuzzy_values.push_back(val);
        update_fuzzy_values_list();
    }
}

void MainWindow::update_values_in_base_operations()
{
    ui->BaseOperationAcomboBox->clear();
    ui->BaseOperationBcomboBox->clear();
    for(size_t i = 0; i < fuzzy_values.size();++i) {
        ui->BaseOperationAcomboBox->addItem(QString::fromStdString(std::to_string(i)));
        ui->BaseOperationBcomboBox->addItem(QString::fromStdString(std::to_string(i)));
    }
}


void MainWindow::on_calculateButton_clicked()
{
    size_t index1 = ui->BaseOperationAcomboBox->currentIndex();
    size_t index2 = ui->BaseOperationBcomboBox->currentIndex();
    if(index1 >= fuzzy_values.size() || index2 >= fuzzy_values.size()) {
        QMessageBox::critical(this, "Ошибка", "Выбран несуществующий элемент");
        return;
    }
    fuzzy_logic::fuzzy_value a = fuzzy_values[index1], b = fuzzy_values[index2], result;
    size_t index_operation = ui->BaseOperatrioncomboBox->currentIndex();
    switch (index_operation) {
    case 0:
        result = a + b;
        break;
    case 1:
        result = a - b;
        break;
    case 2:
        result = a * b;
        break;
    case 3:
        result = a / b;
        break;
    default:
        qDebug() << "Неизвестная арифметическая операция";
    }
    fuzzy_values.push_back(result);
    update_fuzzy_values_list();
}


void MainWindow::update_count_clasters_clasess()
{
    ui->CountClastersClassesComboBox->clear();
    for(size_t i = 1; i <= fuzzy_values.size(); ++i) {
        ui->CountClastersClassesComboBox->addItem(QString::fromStdString(std::to_string(i)));
    }

}

