#include "arithmeticdialog.h"

#include <QFormLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ArithmeticDialog::ArithmeticDialog(std::vector<fuzzy_logic::fuzzy_value> &values,
                                   QWidget *parent)
    : QDialog(parent), values_(values)
{
    setWindowTitle("Арифметические операции");
    resize(520, 220);
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    first_ = new QComboBox(this);
    operation_ = new QComboBox(this);
    second_ = new QComboBox(this);
    operation_->addItems({"+", "-", "*", "/", "сравнить"});
    form->addRow("Первая величина:", first_);
    form->addRow("Операция:", operation_);
    form->addRow("Вторая величина:", second_);
    layout->addLayout(form);
    auto *button = new QPushButton("Выполнить", this);
    layout->addWidget(button);
    result_ = new QLabel(this);
    result_->setWordWrap(true);
    layout->addWidget(result_);
    for (size_t i = 0; i < values_.size(); ++i) {
        const auto interval = values_[i].get_interval();
        const QString text = QString("%1: [%2; %3]")
            .arg(static_cast<int>(i))
            .arg(static_cast<double>(interval.first), 0, 'g', 8)
            .arg(static_cast<double>(interval.second), 0, 'g', 8);
        first_->addItem(text);
        second_->addItem(text);
    }
    connect(button, &QPushButton::clicked, this, &ArithmeticDialog::calculate);
}

void ArithmeticDialog::calculate()
{
    const int first = first_->currentIndex();
    const int second = second_->currentIndex();
    if (first < 0 || second < 0) {
        QMessageBox::warning(this, "Операция", "Нужно выбрать две величины.");
        return;
    }
    try {
        const auto &a = values_[static_cast<size_t>(first)];
        const auto &b = values_[static_cast<size_t>(second)];
        if (operation_->currentIndex() == 4) {
            const int relation = fuzzy_logic::compare(a, b);
            result_->setText(relation < 0 ? "A меньше B" :
                             relation > 0 ? "A больше B" : "Интервалы пересекаются");
            return;
        }
        fuzzy_logic::fuzzy_value value;
        switch (operation_->currentIndex()) {
        case 0: value = a + b; break;
        case 1: value = a - b; break;
        case 2: value = a * b; break;
        case 3: value = a / b; break;
        default: return;
        }
        values_.push_back(value);
        const auto interval = value.get_interval();
        result_->setText(QString("Результат добавлен: [%1; %2]")
                             .arg(static_cast<double>(interval.first), 0, 'g', 8)
                             .arg(static_cast<double>(interval.second), 0, 'g', 8));
    } catch (const std::exception &error) {
        QMessageBox::critical(this, "Ошибка операции", error.what());
    }
}
