#ifndef LQGRAPHICSVIEW_H
#define LQGRAPHICSVIEW_H

#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include "lqgraphicsscene.h"

class LQGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit LQGraphicsView(QWidget *parent=nullptr);
    ~LQGraphicsView();

    void wheelEvent(QWheelEvent* event);
};

#endif // LQGRAPHICSVIEW_H
