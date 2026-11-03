#include "createfuzzyvaluewindow.h"
#include "ui_createfuzzyvaluewindow.h"


CreateFuzzyValueWindow::CreateFuzzyValueWindow(fuzzy_logic::fuzzy_value & val, QWidget *parent, bool read_only)
    : QDialog(parent), value{val}, read_only{read_only}
    , ui(new Ui::CreateFuzzyValueWindow)
{
    ui->setupUi(this);
    ui->AlphasTableWidget->setColumnCount(3);
    ui->AlphasTableWidget->setHorizontalHeaderLabels({"Уровень", "A", "B"});
    ui->AlphasTableWidget->resizeColumnsToContents();


    if (read_only) {
        draw_graphic();
        change_alphas_table();
        restore_alphas_string();

        ui->CreateFuzzyValueButton->setEnabled(false);
        ui->GenerateRandomFuzzyValueButton->setEnabled(false);
        ui->CreateFuzzyValueButton->setEnabled(false);
        ui->AlphasLineEdit->setReadOnly(true);
        ui->WeibullParametrsLineEdit->setReadOnly(true);
    }

}

CreateFuzzyValueWindow::~CreateFuzzyValueWindow()
{
    delete ui;
}

void CreateFuzzyValueWindow::on_GenerateRandomFuzzyValueButton_clicked()
{
    std::string alphas_string = ui->AlphasLineEdit->text().toStdString();
    std::istringstream iss(alphas_string);
    std::vector<long double> alphas((std::istream_iterator<long double>(iss)),
                                    std::istream_iterator<long double>());
    for(size_t i = 0; i < alphas.size(); ++i) {
        if (alphas[i] <= 0 || (i > 1 && i < alphas.size()-1 && (alphas[i-1] > alphas[i] || alphas[i+1] < alphas[i]))) {
            QMessageBox::critical(this, "error", "incorrect alpa");
            return;
        }
    }

    value = fuzzy_logic::generate_random_fuzzy_value(alphas);
    draw_graphic();
    change_alphas_table();
}

void CreateFuzzyValueWindow::draw_graphic() {
    QScatterSeries *scatterSeries = new QScatterSeries();
    QQmlEngine *engine = ui->PlotValueQuickWidget->engine();
    QQmlComponent *pointDelegate = new QQmlComponent(engine);
    pointDelegate->setData(R"(
        import QtQuick 2.0
        Rectangle {
            width: 2
            height: 2
            radius: 2
            color: "red"
        }
    )", QUrl());
    scatterSeries->setPointDelegate(pointDelegate);


    std::vector<std::pair<long double, long double>> alphas_intervals = value.get_alphas_intervals();
    std::vector<long double> alphas = value.get_alphas();
    long double x_min = alphas_intervals[0].first - 0.2;
    long double x_max = alphas_intervals[0].second + 0.2;
    size_t steps = 1000;
    long double x = x_min;
    long double step = (x_max - x_min) / steps;
    for(size_t i = 0; i < steps; ++i) {
        scatterSeries->append(x, value.calc_membership(x));
        x += step;
    }

    auto *axisX = new QValueAxis(this);
    axisX->setRange(x_min, x_max);
    axisX->setTickInterval((x_max - x_min) / 10);
    auto *axisY = new QValueAxis(this);
    axisY->setRange(0, 1.1);
    axisY->setTickInterval(0.1);
    ui->PlotValueQuickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->PlotValueQuickWidget->setInitialProperties({
        {"seriesList", QVariant::fromValue(QList<QObject*>{scatterSeries})},
        {"axisX", QVariant::fromValue<QObject*>(axisX)},
        {"axisY", QVariant::fromValue<QObject*>(axisY)}
    });
    ui->PlotValueQuickWidget->loadFromModule("QtGraphs", "GraphsView");

}


void CreateFuzzyValueWindow::on_CreateFuzzyValueButton_clicked()
{
    std::string alphas_string = ui->AlphasLineEdit->text().toStdString();
    std::istringstream iss(alphas_string);
    std::vector<long double> alphas((std::istream_iterator<long double>(iss)),
                                std::istream_iterator<long double>());
    for(size_t i = 0; i < alphas.size(); ++i) {
        if (alphas[i] <= 0 || (i > 1 && i < alphas.size()-1 && (alphas[i-1] > alphas[i] || alphas[i+1] < alphas[i]))) {
            QMessageBox::critical(this, "error", "incorrect alpa");
            return;
        }
    }
    std::string weibull_string = ui->WeibullParametrsLineEdit->text().toStdString();
    iss = std::istringstream(weibull_string);
    std::vector<long double> weibull_params((std::istream_iterator<long double>(iss)),
                               std::istream_iterator<long double>());
    if (weibull_params.size() != 2 || weibull_params[0] < 0 || weibull_params[0] < 0) {
        qDebug() << weibull_params.size();
        QMessageBox::critical(this, "error", "incorrect weibull params");
        return;
    }
    weibull_distribution_math::weibull_distribution w(weibull_params[0], weibull_params[1]);
    value = fuzzy_logic::fuzzy_value([&w](long double x) {
        return w.membership_function(x);
    }, alphas, w.get_max_x());
    change_alphas_table();
    draw_graphic();
}


void CreateFuzzyValueWindow::on_returnBackButton_clicked()
{

    emit update_fuzzy_values_list_signal();
    ui->PlotValueQuickWidget->close();
    ui->AlphasTableWidget->clear();
    this->close();
}

void CreateFuzzyValueWindow::change_alphas_table()
{
    std::vector<long double> alphas = value.get_alphas();
    ui->AlphasTableWidget->clearContents();
    std::vector<std::pair<long double, long double>> alphas_intervals = value.get_alphas_intervals();
    ui->AlphasTableWidget->setRowCount(alphas_intervals.size());
    for(size_t i = 0; i < alphas_intervals.size(); ++i) {
        ui->AlphasTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(alphas[i], 'g', 2)));
        ui->AlphasTableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(alphas_intervals[i].first, 'g', 6)));
        ui->AlphasTableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(alphas_intervals[i].second, 'g', 7)));
    }
    ui->AlphasTableWidget->resizeColumnsToContents();
}


void CreateFuzzyValueWindow::restore_alphas_string() {
    std::vector<long double> alphas = value.get_alphas();
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    for(size_t i = 0; i < alphas.size() - 1; ++i) {
        oss << alphas[i];
        if (i != alphas.size()-2) {
            oss << " ";
        }
    }
    qDebug() << oss.str();
    ui->AlphasLineEdit->setText(QString::fromStdString(oss.str()));
}

