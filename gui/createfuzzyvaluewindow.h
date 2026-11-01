#ifndef CREATEFUZZYVALUEWINDOW_H
#define CREATEFUZZYVALUEWINDOW_H

#include <QDialog>

namespace Ui {
class CreateFuzzyValueWindow;
}

class CreateFuzzyValueWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CreateFuzzyValueWindow(QWidget *parent = nullptr);
    ~CreateFuzzyValueWindow();

private:
    Ui::CreateFuzzyValueWindow *ui;

};

#endif // CREATEFUZZYVALUEWINDOW_H
