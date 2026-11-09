#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <vector>

#include "fuzzy_logic.hpp"

class ArithmeticDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit ArithmeticDialog(std::vector<fuzzy_logic::fuzzy_value> &values,
                               QWidget *parent = nullptr);

private slots:
    void calculate();

private:
    std::vector<fuzzy_logic::fuzzy_value> &values_;
    QComboBox *first_;
    QComboBox *operation_;
    QComboBox *second_;
    QLabel *result_;
};
