#include "intervalfilterdialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

IntervalFilterDialog::IntervalFilterDialog(
    const std::vector<fuzzy_logic::fuzzy_value> &values, long double k, long double lambda, QWidget *parent)
    : QDialog(parent), values_(values), k_(k), lambda_(lambda)
{
    setWindowTitle("Фильтрация по уровню alpha");
    resize(620, 420);
    auto *layout = new QVBoxLayout(this);
    auto *description = new QLabel(
        "Показываются все исходные интервалы, для которых "
        "оценка принадлежности их центра не ниже α.");
    description->setWordWrap(true);
    layout->addWidget(description);
    auto *form = new QFormLayout();
    alpha_ = new QDoubleSpinBox(this);
    alpha_->setRange(0.0, 1.0);
    alpha_->setSingleStep(0.05);
    alpha_->setValue(0.8);
    form->addRow("Уровень α:", alpha_);
    layout->addLayout(form);
    result_ = new QListWidget(this);
    layout->addWidget(result_);
    connect(alpha_, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &IntervalFilterDialog::update_result);
    update_result();
}

void IntervalFilterDialog::update_result()
{
    result_->clear();
    if (values_.empty()) {
        result_->addItem("Набор пуст");
        return;
    }
    for (size_t index = 0; index < values_.size(); ++index) {
        const auto interval = values_[index].get_interval();
        const long double center = (interval.first + interval.second) / 2.0L;
        const long double membership = fuzzy_logic::generated_membership(center, k_, lambda_);
        if (membership >= alpha_->value()) {
            result_->addItem(QString("%1: [%2; %3], μ=%4")
                                 .arg(static_cast<int>(index))
                                 .arg(static_cast<double>(interval.first), 0, 'g', 8)
                                 .arg(static_cast<double>(interval.second), 0, 'g', 8)
                                 .arg(static_cast<double>(membership), 0, 'f', 3));
        }
    }
    if (result_->count() == 0) result_->addItem("Нет интервалов для выбранного α");
}
