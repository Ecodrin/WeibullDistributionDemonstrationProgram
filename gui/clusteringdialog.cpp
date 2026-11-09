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
    const std::vector<fuzzy_logic::fuzzy_value> &values, QWidget *parent)
    : QDialog(parent), values_(values)
{
    setWindowTitle("Кластеризация нечётких величин");
    resize(650, 420);
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
    metric_->addItems({"Хаусдорф для [A; B]", "Центр и ширина", "Левая граница A",
                      "Правая граница B", "Евклидово расстояние A/B"});
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
    layout->addWidget(graph_, 1);
    connect(button, &QPushButton::clicked, this, &ClusteringDialog::run_clustering);
    connect(metric_, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { run_clustering(); });
    run_clustering();
}

void ClusteringDialog::run_clustering()
{
    if (values_.empty()) return;
    const size_t k = static_cast<size_t>(clusters_->value());
    std::vector<std::vector<long double>> distance(
        values_.size(), std::vector<long double>(values_.size(), 0.0L));
    for (size_t i = 0; i < values_.size(); ++i) {
        for (size_t j = i + 1; j < values_.size(); ++j) {
            const auto a = values_[i].get_interval();
            const auto b = values_[j].get_interval();
            long double d = 0.0L;
            if (metric_->currentIndex() == 0) {
                d = std::max(std::abs(a.first - b.first), std::abs(a.second - b.second));
            } else if (metric_->currentIndex() == 1) {
                const long double center_a = (a.first + a.second) / 2.0L;
                const long double center_b = (b.first + b.second) / 2.0L;
                const long double width_a = a.second - a.first;
                const long double width_b = b.second - b.first;
                const long double dc = center_a - center_b;
                const long double dw = width_a - width_b;
                d = std::sqrt(dc * dc + dw * dw);
            } else if (metric_->currentIndex() == 2) {
                d = std::abs(a.first - b.first);
            } else if (metric_->currentIndex() == 3) {
                d = std::abs(a.second - b.second);
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
        auto *delegate = new QQmlComponent(graph_->engine());
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
    long double max_metric = 0.0L;
    long double min_width = std::numeric_limits<long double>::max();
    long double max_width = std::numeric_limits<long double>::lowest();
    for (size_t i = 0; i < values_.size(); ++i) {
        const int cluster = clustering.labels[i];
        if (cluster < 0 || static_cast<size_t>(cluster) >= series.size()) {
            continue;
        }
        const size_t cluster_index = static_cast<size_t>(cluster);
        const long double metric = distance[i][
            clustering.medoids[cluster_index]];
        const auto interval = values_[i].get_interval();
        const long double width = interval.second - interval.first;
        max_metric = std::max(max_metric, metric);
        min_width = std::min(min_width, width);
        max_width = std::max(max_width, width);
        series[static_cast<size_t>(cluster)]->append(
            static_cast<double>(metric),
            static_cast<double>(width));
    }
    auto *axis_x = new QValueAxis(this);
    axis_x->setTitleText("Метрика до медоида");
    const long double x_padding = std::max(0.0001L, max_metric * 0.1L);
    axis_x->setRange(0.0, static_cast<double>(max_metric + x_padding));
    axis_x->setGridVisible(false);
    axis_x->setSubGridVisible(false);
    auto *axis_y = new QValueAxis(this);
    axis_y->setTitleText("Ширина интервала");
    const long double width_range = max_width - min_width;
    const long double width_padding = std::max(0.0001L, width_range * 0.1L);
    axis_y->setRange(static_cast<double>(min_width - width_padding),
                     static_cast<double>(max_width + width_padding));
    axis_y->setLabelFormat(width_range < 0.01L ? "%.4f" :
                           width_range < 0.1L ? "%.3f" : "%.2f");
    axis_y->setGridVisible(false);
    axis_y->setSubGridVisible(false);
    auto *theme = new QGraphsTheme(this);
    theme->setColorScheme(QGraphsTheme::ColorScheme::Dark);
    theme->setBackgroundColor(QColor("#18181b"));
    theme->setPlotAreaBackgroundColor(QColor("#09090b"));
    theme->setLabelBackgroundVisible(false);
    theme->setLabelTextColor(QColor("#d4d4d8"));
    graph_->setSource(QUrl());
    graph_->setInitialProperties({{"seriesList", QVariant::fromValue(series_list)},
                                  {"axisX", QVariant::fromValue<QObject*>(axis_x)},
                                  {"axisY", QVariant::fromValue<QObject*>(axis_y)},
                                  {"theme", QVariant::fromValue(theme)}});
    graph_->loadFromModule("QtGraphs", "GraphsView");
}
