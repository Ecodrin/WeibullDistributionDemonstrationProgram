#pragma once

#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QQuickWidget>
#include <QSpinBox>
#include <vector>

#include "fuzzy_logic.hpp"

class ClusteringDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit ClusteringDialog(const std::vector<fuzzy_logic::fuzzy_value> &values,
                               long double k, long double lambda,
                               QWidget *parent = nullptr);

private slots:
    void run_clustering();

private:
    void render_clustering(const std::vector<fuzzy_logic::fuzzy_value> &values,
                           QQuickWidget *graph);

    std::vector<fuzzy_logic::fuzzy_value> values_;
    long double k_;
    long double lambda_;
    QSpinBox *clusters_;
    QSpinBox *iterations_;
    QSpinBox *seed_;
    QComboBox *metric_;
    QDoubleSpinBox *alpha_;
    QQuickWidget *graph_;
    QQuickWidget *filtered_graph_;
    QLabel *filtered_status_;
};
