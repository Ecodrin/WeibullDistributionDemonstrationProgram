#include "plotview.h"

PlotView::PlotView(QGraphicsView *view,size_t count_points, QObject *parent)
    : QObject{parent}, m_view{view}, count_points{count_points}
{
    m_scene = new QGraphicsScene(this);
    m_view->setScene(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setBackgroundBrush(Qt::white);
    m_view->viewport()->installEventFilter(this);
}

void PlotView::draw(const fuzzy_logic::fuzzy_value & value) {

    std::vector<long double> alphas = value.get_alphas();
    std::vector<std::pair<long double, long double>> intervals = value.get_alphas_intervals();

    m_scene->clear();
    QRectF r = m_view->mapToScene(m_view->viewport()->rect()).boundingRect()
                   .adjusted(50, 20, -20, -40);

    double xstart = intervals[0].first  - 1.0;
    double xend   = intervals[0].second + 1.0;
    double ystart = -0.2;
    double yend   = 1.2;

    auto pt = [&](double x, double y) -> QPointF {
        return QPointF(
            r.left()   + (x - xstart) / (xend - xstart) * r.width(),
            r.bottom() - (y - ystart) / (yend - ystart) * r.height());
    };

    m_scene->addLine(QLineF(r.bottomLeft(), r.bottomRight()), QPen(Qt::black));
    m_scene->addLine(QLineF(r.bottomLeft(), r.topLeft()),     QPen(Qt::black));

    double step = (xend-xstart) / count_points;
    double x = xstart;

    for (size_t i = 0; i < count_points; ++i) {
        QPointF p = pt(x, value.calc_membership(x));
        m_scene->addEllipse(p.x() - 3, p.y() - 3, 6, 6,
                            QPen(Qt::NoPen), QBrush(Qt::blue));
        x += step;
    }

    m_scene->setSceneRect(m_view->mapToScene(m_view->viewport()->rect()).boundingRect());
}
