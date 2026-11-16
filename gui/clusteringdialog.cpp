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
#include <numeric>

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
    if (values_.empty()) {
        graph_->setSource(QUrl());
        filtered_graph_->setSource(QUrl());
        filtered_status_->clear();
        return;
    }

    std::vector<size_t> filtered_indices;
    const long double alpha = alpha_->value();
    for (size_t i = 0; i < values_.size(); ++i) {
        const auto interval = values_[i].get_interval();
        const long double center = (interval.first + interval.second) / 2.0L;
        if (fuzzy_logic::generated_membership(center, k_, lambda_) >= alpha) {
            filtered_indices.push_back(i);
        }
    }
    filtered_status_->setText(
        QString("Отобрано интервалов: %1 из %2 (α = %3)")
            .arg(static_cast<int>(filtered_indices.size()))
            .arg(static_cast<int>(values_.size()))
            .arg(static_cast<double>(alpha), 0, 'f', 4));

    auto build_distance_matrix = [this](
                                     const std::vector<size_t> &indices) {
        std::vector<std::vector<long double>> result(
            indices.size(), std::vector<long double>(indices.size(), 0.0L));
        for (size_t i = 0; i < indices.size(); ++i) {
            for (size_t j = i + 1; j < indices.size(); ++j) {
                const auto a = values_[indices[i]].get_interval();
                const auto b = values_[indices[j]].get_interval();
                long double d = 0.0L;
                switch (metric_->currentIndex()) {
                case 0:
                    d = std::max(std::abs(a.first - b.first),
                                 std::abs(a.second - b.second));
                    break;
                case 1:
                    d = std::abs((a.second - a.first) -
                                 (b.second - b.first));
                    break;
                case 2: {
                    const long double wa = a.second - a.first;
                    const long double wb = b.second - b.first;
                    d = std::abs(wa * wa - wb * wb);
                    break;
                }
                default: {
                    const long double da = a.first - b.first;
                    const long double db = a.second - b.second;
                    d = std::sqrt(da * da + db * db);
                    break;
                }
                }
                result[i][j] = result[j][i] = d;
            }
        }
        return result;
    };

    std::vector<size_t> all_indices(values_.size());
    std::iota(all_indices.begin(), all_indices.end(), size_t{0});
    const auto all_distance = build_distance_matrix(all_indices);
    const size_t all_k = std::min(static_cast<size_t>(clusters_->value()),
                                  all_indices.size());
    const auto all_clustering = fuzzy_logic::k_medoids(
        all_distance, all_k, static_cast<size_t>(iterations_->value()),
        seed_->value());
    long double min_center = std::numeric_limits<long double>::max();
    long double max_center = std::numeric_limits<long double>::lowest();
    long double max_y = 0.0L;
    for (const auto &value : values_) {
        const auto interval = value.get_interval();
        const long double center = (interval.first + interval.second) / 2.0L;
        min_center = std::min(min_center, center);
        max_center = std::max(max_center, center);
        const long double width = interval.second - interval.first;
        const long double y = metric_->currentIndex() == 1
                                  ? width
                                  : metric_->currentIndex() == 2
                                      ? width * width
                                      : 0.0L;
        max_y = std::max(max_y, y);
    }
    if (metric_->currentIndex() == 0 ||
        metric_->currentIndex() == 3) {
        for (size_t i = 0; i < all_indices.size(); ++i) {
            const int cluster = all_clustering.labels[i];
            if (cluster >= 0 &&
                static_cast<size_t>(cluster) < all_clustering.medoids.size()) {
                max_y = std::max(
                    max_y,
                    all_distance[i][static_cast<size_t>(
                        all_clustering.medoids[static_cast<size_t>(cluster)])]);
            }
        }
    }
    render_clustering(all_indices, all_distance, all_clustering.medoids,
                      all_clustering.labels, {}, min_center, max_center,
                      max_y, graph_);

    if (filtered_indices.empty()) {
        filtered_graph_->setSource(QUrl());
        return;
    }
    const auto filtered_distance = build_distance_matrix(filtered_indices);
    const size_t filtered_k = std::min(static_cast<size_t>(clusters_->value()),
                                       filtered_indices.size());
    const auto filtered_clustering = fuzzy_logic::k_medoids(
        filtered_distance, filtered_k, static_cast<size_t>(iterations_->value()),
        seed_->value());
    std::vector<std::vector<size_t>> color_counts(
        filtered_k, std::vector<size_t>(all_k, 0));
    for (size_t filtered_cluster = 0; filtered_cluster < filtered_k;
         ++filtered_cluster) {
        for (size_t i = 0; i < filtered_indices.size(); ++i) {
            if (filtered_clustering.labels[i] !=
                static_cast<int>(filtered_cluster)) {
                continue;
            }
            const int original_cluster = all_clustering.labels[
                filtered_indices[i]];
            if (original_cluster >= 0 &&
                static_cast<size_t>(original_cluster) < all_k) {
                ++color_counts[filtered_cluster][
                    static_cast<size_t>(original_cluster)];
            }
        }
    }

    std::vector<int> filtered_color_indices(filtered_k, -1);
    std::vector<bool> used_colors(all_k, false);
    for (size_t assigned = 0; assigned < filtered_k; ++assigned) {
        size_t selected_cluster = assigned;
        size_t selected_color = 0;
        size_t selected_count = 0;
        bool found_unused = false;
        for (size_t filtered_cluster = 0; filtered_cluster < filtered_k;
             ++filtered_cluster) {
            if (filtered_color_indices[filtered_cluster] >= 0) continue;
            for (size_t color = 0; color < all_k; ++color) {
                if (used_colors[color]) continue;
                if (!found_unused ||
                    color_counts[filtered_cluster][color] > selected_count) {
                    selected_cluster = filtered_cluster;
                    selected_color = color;
                    selected_count =
                        color_counts[filtered_cluster][color];
                    found_unused = true;
                }
            }
        }
        filtered_color_indices[selected_cluster] =
            static_cast<int>(selected_color);
        used_colors[selected_color] = true;
    }
    render_clustering(filtered_indices, filtered_distance,
                      filtered_clustering.medoids,
                      filtered_clustering.labels, filtered_color_indices,
                      min_center, max_center, max_y, filtered_graph_);
}

void ClusteringDialog::render_clustering(
    const std::vector<size_t> &indices,
    const std::vector<std::vector<long double>> &distance,
    const std::vector<int> &medoids,
    const std::vector<int> &labels,
    const std::vector<int> &color_indices,
    long double min_center,
    long double max_center,
    long double max_y,
    QQuickWidget *graph)
{
    if (indices.empty()) {
        graph->setSource(QUrl());
        return;
    }

    const size_t k = medoids.size();
    const QList<QColor> colors{
        QColor("#377eb8"), QColor("#4daf4a"), QColor("#e41a1c"),
        QColor("#ff7f00"), QColor("#984ea3"), QColor("#a65628"),
    };

    QList<QObject *> series_list;
    std::vector<QScatterSeries *> series;
    series.reserve(k);

    for (size_t cluster = 0; cluster < k; ++cluster) {
        const size_t color_index = color_indices.empty()
                                       ? cluster
                                       : static_cast<size_t>(
                                             color_indices[cluster]);
        const QColor color = colors[color_index % colors.size()];
        auto *s = new QScatterSeries();
        s->setName(QString("Кластер %1").arg(static_cast<int>(cluster + 1)));
        s->setColor(color);
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
        s->setPointDelegate(delegate);
        series.push_back(s);
        series_list.append(s);
    }

    auto compute_point = [&](size_t position, long double &x, long double &y,
                             size_t &cluster_out) -> bool {
        const int cluster = labels[position];
        if (cluster < 0 || static_cast<size_t>(cluster) >= series.size()) {
            return false;
        }
        const size_t c = static_cast<size_t>(cluster);
        const size_t idx = indices[position];
        const auto interval = values_[idx].get_interval();
        x = (interval.first + interval.second) / 2.0L;
        const long double width = interval.second - interval.first;
        const long double metric_distance =
            distance[position][static_cast<size_t>(medoids[c])];
        y = metric_->currentIndex() == 1
                ? width
                : metric_->currentIndex() == 2 ? width * width : metric_distance;
        cluster_out = c;
        return true;
    };

    for (size_t i = 0; i < indices.size(); ++i) {
        long double x, y;
        size_t c;
        if (!compute_point(i, x, y, c)) {
            continue;
        }
        series[c]->append(static_cast<double>(x), static_cast<double>(y));
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
    const long double metric_padding = std::max(0.0001L, max_y * 0.1L);
    axis_y->setRange(0.0, static_cast<double>(max_y + metric_padding));
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