#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "clusteringdialog.h"
#include "intervalfilterdialog.h"
#include "arithmeticdialog.h"

#include <QMessageBox>
#include <QQuickWidget>
#include <QScatterSeries>
#include <QValueAxis>
#include <QGraphsTheme>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QUrl>
#include <QVariant>
#include <QList>

#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->OverallGraphQuickWidget->setClearColor(QColor("#18181b"));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_fuzzy_values_list()
{
    ui->FuzzyValueslistWidget->clear();
    for(size_t i = 0; i < fuzzy_values.size();++i) {
        const auto interval = fuzzy_values[i].get_interval();
        auto *item = new QListWidgetItem(
            QString("%1: [%2; %3]")
                .arg(static_cast<int>(i))
                .arg(static_cast<double>(interval.first), 0, 'g', 8)
                .arg(static_cast<double>(interval.second), 0, 'g', 8));
        item->setData(Qt::UserRole, static_cast<int>(i));
        ui->FuzzyValueslistWidget->addItem(item);
    }
    update_overall_graph();
}

void MainWindow::on_GenerateFuzzyValuesButton_clicked()
{
    size_t count_generate = ui->CountGenerateFuzzyValuesspinBox->value();
    for(size_t i = 0; i < count_generate; ++i) {
        fuzzy_logic::fuzzy_value v = fuzzy_logic::generate_random_fuzzy_value(ui->KSpinBox->value(), ui->LambdadoubleSpinBox->value());
        fuzzy_values.push_back(v);
    }

    update_fuzzy_values_list();
}

void MainWindow::on_ClearFuzzyValuesButton_clicked()
{
    fuzzy_values.clear();
    update_fuzzy_values_list();
}

void MainWindow::on_ShowAlphaCutButton_clicked()
{
    IntervalFilterDialog dialog(fuzzy_values, ui->KSpinBox->value(), ui->LambdadoubleSpinBox->value(), this);
    dialog.exec();
}


void MainWindow::on_OpenArithmeticButton_clicked()
{
    if (fuzzy_values.empty()) {
        QMessageBox::information(this, "Операции", "Сначала сгенерируйте интервалы.");
        return;
    }
    ArithmeticDialog dialog(fuzzy_values, this);
    dialog.exec();
    update_fuzzy_values_list();
}




void MainWindow::update_overall_graph()
{
    if (fuzzy_values.empty()) {
        ui->OverallGraphQuickWidget->setSource(QUrl());
        return;
    }

    long double maximum = 0.0L;
    for (const auto &value : fuzzy_values) {
        maximum = std::max(maximum, value.right());
    }
    if (maximum <= 0.0L) {
        return;
    }

    auto *series = new QScatterSeries();
    QQmlEngine *engine = ui->OverallGraphQuickWidget->engine();
    auto *delegate = new QQmlComponent(engine);
    delegate->setData(R"(
        import QtQuick 2.0
        Rectangle {
            width: 4
            height: 4
            radius: 2
            color: "#22c55e"
        }
    )", QUrl());
    series->setPointDelegate(delegate);

    constexpr size_t steps = 400;
    for (size_t step = 0; step <= steps; ++step) {
        const long double x = maximum * static_cast<long double>(step) / steps;
        series->append(static_cast<double>(x),
                       static_cast<double>(fuzzy_logic::generated_membership(x, ui->KSpinBox->value(), ui->LambdadoubleSpinBox->value())));
    }

    auto *axisX = new QValueAxis(this);
    axisX->setRange(0.0, static_cast<double>(maximum));
    axisX->setTitleText("Значение x");
    auto *axisY = new QValueAxis(this);
    axisY->setRange(0.0, 1.05);
    axisY->setTickInterval(0.1);
    axisY->setTitleText("Функция принадлежности");

    auto *theme = new QGraphsTheme(this);
    theme->setColorScheme(QGraphsTheme::ColorScheme::Dark);
    theme->setBackgroundColor(QColor("#18181b"));
    theme->setPlotAreaBackgroundColor(QColor("#09090b"));
    theme->setLabelBackgroundVisible(false);
    theme->setLabelTextColor(QColor("#d4d4d8"));

    ui->OverallGraphQuickWidget->setInitialProperties({
        {"seriesList", QVariant::fromValue(QList<QObject*>{series})},
        {"axisX", QVariant::fromValue<QObject*>(axisX)},
        {"axisY", QVariant::fromValue<QObject*>(axisY)},
        {"theme", QVariant::fromValue(theme)}
    });
    ui->OverallGraphQuickWidget->loadFromModule("QtGraphs", "GraphsView");
}

void MainWindow::on_CreateClastersPushButton_clicked()
{
    if (fuzzy_values.empty()) {
        QMessageBox::information(this, "Кластеризация", "Сначала сгенерируйте интервалы.");
        return;
    }
    std::vector<fuzzy_logic::fuzzy_value> selected;
    for (const auto *item : ui->FuzzyValueslistWidget->selectedItems()) {
        const int index = item->data(Qt::UserRole).toInt();
        if (index >= 0 && static_cast<size_t>(index) < fuzzy_values.size()) {
            selected.push_back(fuzzy_values[static_cast<size_t>(index)]);
        }
    }
    if (selected.empty()) {
        selected = fuzzy_values;
    }
    ClusteringDialog dialog(selected, ui->KSpinBox->value(),
                            ui->LambdadoubleSpinBox->value(), this);
    dialog.exec();
}

void MainWindow::on_SaveLogPushButton_clicked()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        "Сохранить в log file",
        "",
        "Все файлы (*.*)"
        );
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        std::ostringstream oss;
        for(const auto & el : fuzzy_values) {
            oss << el << "\n";
        }
        out << QString::fromStdString(oss.str());
        file.close();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось записать в файл");
    }
}

