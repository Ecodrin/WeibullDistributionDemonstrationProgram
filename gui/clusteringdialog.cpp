#include "clusteringdialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScatterSeries>
#include <QValueAxis>
#include <QGraphsTheme>
#include <QList>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>
#include <QColor>
#include <algorithm>
#include <cmath>
#include <limits>

ClusteringDialog::ClusteringDialog(
    const std::vector<fuzzy_logic::fuzzy_value> &values, long double k,
    long double lambda, QWidget *parent)
    : QDialog(parent), values_(values), k_(k), lambda_(lambda)
{
    setWindowTitle("Кластеризация нечётких величин");
    resize(900, 800);
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    clusters_ = new QSpinBox(this);
    clusters_->setRange(1, std::max(1, static_cast<int>(values_.size())));
    clusters_->setValue(std::min(3, clusters_->maximum()));
    iterations_ = new QSpinBox(this);
    iterations_->setRange(1, 1000);
    iterations_->setValue(100);
    seed_ = new QSpinBox(this);
    seed_->setRange(0, 1000000);
    seed_->setValue(42);
    metric_ = new QComboBox(this);
    metric_->addItems({"Хаусдорф для [A; B]", "Ширина интервала B - A",
                      "Квадрат ширины (B - A)^2", "Евклидово расстояние A/B"});
    alpha_ = new QDoubleSpinBox(this);
    alpha_->setRange(0.0, 1.0);
    alpha_->setDecimals(4);
    alpha_->setSingleStep(0.0001);
    alpha_->setValue(0.8000);
    form->addRow("Количество кластеров:", clusters_);
    form->addRow("Максимум итераций:", iterations_);
    form->addRow("Seed:", seed_);
    form->addRow("Метрика:", metric_);
    layout->addLayout(form);
    auto *button = new QPushButton("Запустить кластеризацию", this);
    layout->addWidget(button);
    graph_ = new QQuickWidget(this);
    graph_->setMinimumSize(0, 180);
    graph_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph_->setClearColor(QColor("#18181b"));
    graph_->setResizeMode(QQuickWidget::SizeRootObjectToView);
    auto *all_values_title = new QLabel("Кластеризация всего набора", this);
    all_values_title->setStyleSheet("font-size: 15px; font-weight: bold;");
    layout->addWidget(all_values_title);
    layout->addWidget(graph_, 1);
    auto *filtered_title = new QLabel(
        "Кластеризация после фильтрации по α", this);
    filtered_title->setStyleSheet("font-size: 15px; font-weight: bold;");
    layout->addWidget(filtered_title);
    auto *alpha_form = new QFormLayout();
    alpha_form->addRow("Уровень α для второго графика:", alpha_);
    layout->addLayout(alpha_form);
    filtered_status_ = new QLabel(this);
    layout->addWidget(filtered_status_);
    filtered_graph_ = new QQuickWidget(this);
    filtered_graph_->setMinimumSize(0, 180);
    filtered_graph_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    filtered_graph_->setClearColor(QColor("#18181b"));
    filtered_graph_->setResizeMode(QQuickWidget::SizeRootObjectToView);
    layout->addWidget(filtered_graph_, 1);
    connect(button, &QPushButton::clicked, this, &ClusteringDialog::run_clustering);
    connect(metric_, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { run_clustering(); });
    connect(alpha_, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, [this](double) { run_clustering(); });
    run_clustering();
}

void ClusteringDialog::run_clustering()
{
    render_clustering(values_, graph_);

    std::vector<fuzzy_logic::fuzzy_value> filtered;
    const long double alpha = alpha_->value();
    for (const auto &value : values_) {
        const auto interval = value.get_interval();
        const long double center = (interval.first + interval.second) / 2.0L;
        if (fuzzy_logic::generated_membership(center, k_, lambda_) >= alpha) {
            filtered.push_back(value);
        }
    }
    filtered_status_->setText(
        QString("Отобрано интервалов: %1 из %2 (α = %3)")
            .arg(static_cast<int>(filtered.size()))
            .arg(static_cast<int>(values_.size()))
            .arg(static_cast<double>(alpha), 0, 'f', 4));
    render_clustering(filtered, filtered_graph_);
}

void ClusteringDialog::render_clustering(
    const std::vector<fuzzy_logic::fuzzy_value> &values, QQuickWidget *graph)
{
    if (values.empty()) {
        graph->setSource(QUrl());
        return;
    }
    const size_t k = std::min(static_cast<size_t>(clusters_->value()), values.size());
    std::vector<std::vector<long double>> distance(
        values.size(), std::vector<long double>(values.size(), 0.0L));
    for (size_t i = 0; i < values.size(); ++i) {
        for (size_t j = i + 1; j < values.size(); ++j) {
            const auto a = values[i].get_interval();
            const auto b = values[j].get_interval();
            long double d = 0.0L;
            if (metric_->currentIndex() == 0) {
                d = std::max(std::abs(a.first - b.first), std::abs(a.second - b.second));
            } else if (metric_->currentIndex() == 1) {
                const long double width_a = a.second - a.first;
                const long double width_b = b.second - b.first;
                d = std::abs(width_a - width_b);
            } else if (metric_->currentIndex() == 2) {
                const long double width_a = a.second - a.first;
                const long double width_b = b.second - b.first;
                d = std::abs(width_a * width_a - width_b * width_b);
            } else {
                const long double da = a.first - b.first;
                const long double db = a.second - b.second;
                d = std::sqrt(da * da + db * db);
            }
            distance[i][j] = distance[j][i] = d;
        }
    }
    const auto clustering = fuzzy_logic::k_medoids(
        distance, k, static_cast<size_t>(iterations_->value()), seed_->value());
    const QList<QColor> colors{
        QColor("#ef4444"), QColor("#22c55e"), QColor("#3b82f6"),
        QColor("#eab308"), QColor("#a855f7"), QColor("#14b8a6")
    };
    QList<QObject *> series_list;
    std::vector<QScatterSeries *> series;
    series.reserve(k);
    for (size_t cluster = 0; cluster < k; ++cluster) {
        auto *cluster_series = new QScatterSeries();
        cluster_series->setName(QString("Кластер %1").arg(static_cast<int>(cluster + 1)));
        cluster_series->setColor(colors[cluster % colors.size()]);
        const QColor color = colors[cluster % colors.size()];
        auto *delegate = new QQmlComponent(graph->engine());
        delegate->setData(QString(R"(
            import QtQuick 2.0
            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: "%1"
            }
        )").arg(color.name()).toUtf8(), QUrl());
        cluster_series->setPointDelegate(delegate);
        series.push_back(cluster_series);
        series_list.append(cluster_series);
    }
    long double min_center = std::numeric_limits<long double>::max();
    long double max_center = std::numeric_limits<long double>::lowest();
    long double max_metric = 0.0L;
    for (size_t i = 0; i < values.size(); ++i) {
        const int cluster = clustering.labels[i];
        if (cluster < 0 || static_cast<size_t>(cluster) >= series.size()) {
            continue;
        }
        const size_t cluster_index = static_cast<size_t>(cluster);
        const auto interval = values[i].get_interval();
        const long double center = (interval.first + interval.second) / 2.0L;
        const long double width = interval.second - interval.first;
        const long double metric = distance[i][clustering.medoids[cluster_index]];
        const long double y_value = metric_->currentIndex() == 1
                                        ? width
                                        : metric_->currentIndex() == 2
                                            ? width * width
                                            : metric;
        min_center = std::min(min_center, center);
        max_center = std::max(max_center, center);
        max_metric = std::max(max_metric, y_value);
        series[static_cast<size_t>(cluster)]->append(
            static_cast<double>(center),
            static_cast<double>(y_value));
    }
    auto *axis_x = new QValueAxis(this);
    axis_x->setTitleText("Центр интервала (A + B) / 2");
    const long double center_range = max_center - min_center;
    const long double center_padding = std::max(0.0001L, center_range * 0.1L);
    axis_x->setRange(static_cast<double>(min_center - center_padding),
                     static_cast<double>(max_center + center_padding));
    axis_x->setLabelFormat(center_range < 0.01L ? "%.4f" :
                           center_range < 0.1L ? "%.3f" : "%.2f");
    axis_x->setGridVisible(false);
    axis_x->setSubGridVisible(false);
    auto *axis_y = new QValueAxis(this);
    axis_y->setTitleText(metric_->currentIndex() == 1
                             ? "Ширина интервала B - A"
                             : metric_->currentIndex() == 2
                                 ? "Квадрат ширины интервала (B - A)^2"
                                 : "Метрика до медоида");
    const long double metric_padding = std::max(0.0001L, max_metric * 0.1L);
    axis_y->setRange(0.0, static_cast<double>(max_metric + metric_padding));
    axis_y->setGridVisible(false);
    axis_y->setSubGridVisible(false);
    auto *theme = new QGraphsTheme(this);
    theme->setColorScheme(QGraphsTheme::ColorScheme::Dark);
    theme->setBackgroundColor(QColor("#18181b"));
    theme->setPlotAreaBackgroundColor(QColor("#09090b"));
    theme->setLabelBackgroundVisible(false);
    theme->setLabelTextColor(QColor("#d4d4d8"));
    graph->setSource(QUrl());
    graph->setInitialProperties({{"seriesList", QVariant::fromValue(series_list)},
                                 {"axisX", QVariant::fromValue<QObject*>(axis_x)},
                                 {"axisY", QVariant::fromValue<QObject*>(axis_y)},
                                 {"theme", QVariant::fromValue(theme)}});
    graph->loadFromModule("QtGraphs", "GraphsView");
}
