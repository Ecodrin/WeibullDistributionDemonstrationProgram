#ifndef PLOTVIEW_H
#define PLOTVIEW_H

#include <QObject>
#include <QGraphicsView>

#include "fuzzy_logic.hpp"

class PlotView : public QObject
{
    Q_OBJECT
public:
    explicit PlotView(QGraphicsView *view, size_t count_points=10000, QObject *parent = nullptr);
    PlotView() = default;
    void draw(const fuzzy_logic::fuzzy_value & value);

private:
    QGraphicsView *m_view;
    QGraphicsScene *m_scene;

    size_t count_points;
};

#endif // PLOTVIEW_H
