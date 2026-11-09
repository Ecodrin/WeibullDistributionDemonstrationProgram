#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include <QListWidget>
#include <vector>

#include "fuzzy_logic.hpp"

class IntervalFilterDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit IntervalFilterDialog(const std::vector<fuzzy_logic::fuzzy_value> &values,
                                   long double k, long double lambda, QWidget *parent = nullptr);

private slots:
    void update_result();

private:
    std::vector<fuzzy_logic::fuzzy_value> values_;
    long double k_;
    long double lambda_;
    QDoubleSpinBox *alpha_;
    QListWidget *result_;
};
