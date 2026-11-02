#ifndef CREATEFUZZYVALUEWINDOW_H
#define CREATEFUZZYVALUEWINDOW_H

#include <QDialog>
#include <QMessageBox>

#include <sstream>

#include "fuzzy_logic.hpp"
#include "plotview.h"

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

signals:
    void update_fuzzy_values_list_signal();

private:
    Ui::CreateFuzzyValueWindow *ui;
    fuzzy_logic::fuzzy_value &value;
    PlotView *plot_view;
    bool read_only;

    void draw_graphic();
    void change_alphas_table();
};

#endif // CREATEFUZZYVALUEWINDOW_H
