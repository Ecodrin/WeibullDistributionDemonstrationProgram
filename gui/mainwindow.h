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



    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;

    std::vector<fuzzy_logic::fuzzy_value> fuzzy_values;

};
#endif // MAINWINDOW_H
