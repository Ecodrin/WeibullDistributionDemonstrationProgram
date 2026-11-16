#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>

#include <QMainWindow>
#include <QFileDialog>
#include <sstream>

#include "fuzzy_logic.hpp"


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
    void update_fuzzy_values_list();
    void on_GenerateFuzzyValuesButton_clicked();
    void on_ClearFuzzyValuesButton_clicked();
    void on_OpenArithmeticButton_clicked();
    void on_CreateClastersPushButton_clicked();
    void on_ShowAlphaCutButton_clicked();

    void on_SaveLogPushButton_clicked();

private:
    void update_overall_graph();

    Ui::MainWindow *ui;

    std::vector<fuzzy_logic::fuzzy_value> fuzzy_values;


};
#endif // MAINWINDOW_H
