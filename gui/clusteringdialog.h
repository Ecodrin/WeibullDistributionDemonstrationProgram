#pragma once

#include <QDialog>
#include <QComboBox>
#include <QQuickWidget>
#include <QSpinBox>
#include <vector>

#include "fuzzy_logic.hpp"

class ClusteringDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit ClusteringDialog(const std::vector<fuzzy_logic::fuzzy_value> &values,
                               QWidget *parent = nullptr);

private slots:
    void run_clustering();

private:
    std::vector<fuzzy_logic::fuzzy_value> values_;
    QSpinBox *clusters_;
    QSpinBox *iterations_;
    QSpinBox *seed_;
    QComboBox *metric_;
    QQuickWidget *graph_;
};
