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

void MainWindow::draw_clasters(const std::vector<fuzzy_logic::fuzzy_value> & values) {
    ui->ClasterQuickWidget->setClearColor(QColor("#18181b"));

    QScatterSeries *scatterSeries = new QScatterSeries();
    QQmlEngine *engine = ui->ClasterQuickWidget->engine();
    QQmlComponent *pointDelegate = new QQmlComponent(engine);

    pointDelegate->setData(R"(
        import QtQuick 2.0
        Rectangle {
        width: 9
        height: 9
        radius: 5
        color: "#ff0000"
        }
        )", QUrl());
    scatterSeries->setPointDelegate(pointDelegate);

    if(values.empty()) {
        return;
    }

    size_t count_clasters = ui->CountClastersClassesComboBox->currentIndex() + 1;

    if(count_clasters <= 0) {
        return;
    }
    std::vector<long double> weights = fuzzy_logic::calculate_weights(fuzzy_values[0].get_alphas());
    std::vector<std::vector<long double>> dist_matrix = fuzzy_logic::cals_distance_matrix(fuzzy_values, weights);
    fuzzy_logic::k_mediods_res clasters = fuzzy_logic::k_medoids(dist_matrix, count_clasters, 100, 42);


    for(size_t i = 0; i < fuzzy_values.size(); ++i) {
        scatterSeries->append(i,clasters.labels[i] > 0 ? clasters.labels[i] : count_clasters);
    }


    auto *axisX = new QValueAxis(this);
    axisX->setTickInterval(1);
    axisX->setRange(-0.5, values.size()-0.5);
    axisX->setTickAnchor(0);
    axisX->setLabelFormat("%.0f");
    axisX->setTitleText("Нечеткие значения");
    axisX->setLabelsAngle(270.0);


    auto *axisY = new QValueAxis(this);
    axisY->setTickInterval(1);
    axisY->setTickAnchor(0);
    axisY->setLabelFormat("%.0f");
    axisY->setRange(0.5, count_clasters + 0.5);
    axisY->setTitleText("Кластеры");

    QGraphsTheme *theme = new QGraphsTheme(this);

    theme->setColorScheme(QGraphsTheme::ColorScheme::Dark);

    theme->setBackgroundColor(QColor("#18181b"));
    theme->setBackgroundVisible(true);

    theme->setPlotAreaBackgroundColor(QColor("#09090b"));
    theme->setPlotAreaBackgroundVisible(true);

    theme->setLabelBackgroundVisible(false);
    theme->setLabelTextColor(QColor("#d4d4d8"));
    ui->ClasterQuickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    ui->ClasterQuickWidget->setInitialProperties({
        {"seriesList", QVariant::fromValue(QList<QObject*>{scatterSeries})},
        {"axisX", QVariant::fromValue<QObject*>(axisX)},
        {"axisY", QVariant::fromValue<QObject*>(axisY)},
        {"theme", QVariant::fromValue(theme)}
    });

    ui->ClasterQuickWidget->loadFromModule("QtGraphs", "GraphsView");

}



void MainWindow::on_CreateClastersPushButton_clicked()
{
    QList<QListWidgetItem*> items = ui->FuzzyValueslistWidget->selectedItems();
    std::vector<fuzzy_logic::fuzzy_value> values;
    for(size_t i = 0; i < items.size(); ++i) {
        values.push_back(fuzzy_values[items[i]->text().toULong()]);
    }
    try {
        draw_clasters(values);
    } catch(const std::exception & err) {
        QMessageBox::critical(this, "error", "Выбраны некорректные нечеткие значения: разные альфа уровни");
    }

}

