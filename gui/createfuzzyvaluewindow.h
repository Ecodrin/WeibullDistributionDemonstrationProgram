#ifndef CREATEFUZZYVALUEWINDOW_H
#define CREATEFUZZYVALUEWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QQuickWidget>
#include <QScatterSeries>
#include <QValueAxis>
#include <QList>
#include <QVariant>
#include <QXYSeries>
#include <QFileDialog>

#include <sstream>
#include <format>
#include <string>

#include "fuzzy_logic.hpp"

namespace Ui {
class CreateFuzzyValueWindow;
}

class CreateFuzzyValueWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CreateFuzzyValueWindow(fuzzy_logic::fuzzy_value & val,QWidget *parent = nullptr,bool read_only = false);
    ~CreateFuzzyValueWindow();

private slots:
    void on_GenerateRandomFuzzyValueButton_clicked();

    void on_CreateFuzzyValueButton_clicked();

    void on_returnBackButton_clicked();

    void on_AlphasSpinBox_valueChanged(int arg1);

    void on_SaveLogFilePushButton_clicked();

signals:
    void update_fuzzy_values_list_signal();

private:
    Ui::CreateFuzzyValueWindow *ui;
    fuzzy_logic::fuzzy_value &value;
    bool read_only;

    void draw_graphic();
    void change_alphas_table();
    void restore_alphas_string();
};

#endif // CREATEFUZZYVALUEWINDOW_H
