#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>

#include <QListWidget>
#include <QMainWindow>
#include "createfuzzyvaluewindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_CreateFuzzyValueButton_clicked();
    void update_fuzzy_values_list();
    void add_fuzzy_value_to_list(fuzzy_logic::fuzzy_value val);



    void on_GenerateFuzzyValuesButton_clicked();

    void on_FuzzyValueslistWidget_itemDoubleClicked(QListWidgetItem *item);
    void update_values_in_base_operations();

    void on_calculateButton_clicked();
    void update_count_clasters_clasess();

private:
    Ui::MainWindow *ui;

    std::vector<fuzzy_logic::fuzzy_value> fuzzy_values;


};
#endif // MAINWINDOW_H
