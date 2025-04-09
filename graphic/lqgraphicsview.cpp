#include "lqgraphicsview.h"
#include <QWheelEvent>
#include <QGraphicsItem>

LQGraphicsView::LQGraphicsView(QWidget *parent):QGraphicsView(parent)
{
    setMouseTracking(true);
}

LQGraphicsView::~LQGraphicsView()
{

}

void LQGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        QPoint pp1 = event->position().toPoint();
        QPointF scenePos = mapToScene(pp1.x(),pp1.y());  // 获取鼠标在场景中的位置（缩放前）
        QPoint angleDelta = event->angleDelta();
        if (angleDelta.y() > 0) { // 滚轮向上滚动，放大
            scale(1.1,1.1);
        } else if (angleDelta.y() < 0) { // 滚轮向下滚动，缩小
            scale(0.9,0.9);
        }

        QPoint pp2 = event->position().toPoint();
        QPointF newScenePos = mapToScene(pp2.x(),pp2.y());  // 获取缩放后的鼠标位置
        QPointF offset = newScenePos - scenePos;  // 计算偏移量
        translate(offset.x(), offset.y());  // 平移视图，保持鼠标在同一位置
    }
    else{

            qreal arg = 0;
            if (event->angleDelta().y() > 0) {
                arg = 1;
            } else {
                arg = -1;
            }
            static_cast<LQGraphicsScene*>(scene())->rect_item_roate(arg);
//            QList<QGraphicsItem *> sitems = scene()->selectedItems();
//            for(int i=0;i!=sitems.size();i++)
//            {
//                if(sitems[i]->type() == LQGraphicsRectItem::Type)
//                    sitems[i]->setRotation(sitems[i]->rotation() + arg*1);
//            }

    }
    // 事件传递
    event->accept();
}
