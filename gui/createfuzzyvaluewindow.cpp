#include "createfuzzyvaluewindow.h"
#include "ui_createfuzzyvaluewindow.h"

CreateFuzzyValueWindow::CreateFuzzyValueWindow(fuzzy_logic::fuzzy_value & val, QWidget *parent, bool read_only)
    : QDialog(parent), value{val}, read_only{read_only}
    , ui(new Ui::CreateFuzzyValueWindow)
{
    ui->setupUi(this);
    value = fuzzy_logic::generate_random_fuzzy_value();
    plot_view = new PlotView(ui->graphicsView, 10000, this);
    if (read_only) {
        std::ostringstream oss;
        std::vector<long double> alphas = value.get_alphas();
        oss << std::fixed << std::setprecision(2);
        for (size_t i = 0; i < alphas.size(); ++i) {
            oss << alphas[i];
            if (i < alphas.size() - 1) {
                oss << " ";
            }
        }
        ui->AlphasLineEdit->textEdited(QString::fromStdString(oss.str()));
        oss.clear();
        ui->WeibullParametrsLineEdit->clear();
        change_alphas_table();
    }
    ui->AlphasTableWidget->setColumnCount(3);
    ui->AlphasTableWidget->setHorizontalHeaderLabels({"Уровень", "A", "B"});
    ui->AlphasTableWidget->resizeColumnsToContents();

}

CreateFuzzyValueWindow::~CreateFuzzyValueWindow()
{
    delete plot_view;
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
    // draw_graphic();
    change_alphas_table();
}

void CreateFuzzyValueWindow::draw_graphic() {
    plot_view->draw(value);
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
}


void CreateFuzzyValueWindow::on_returnBackButton_clicked()
{

    emit update_fuzzy_values_list_signal();
    this->close();
}

void CreateFuzzyValueWindow::change_alphas_table() {
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

