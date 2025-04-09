#ifndef LQGRAPHICSITEM_H
#define LQGRAPHICSITEM_H

#include <QObject>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPolygonItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QLineF>
#include <QtMath>
#include "../global_data.h"

class LQGraphicsEllipseItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    LQGraphicsEllipseItem(QGraphicsItem *parent = nullptr)
        : QObject(nullptr), QGraphicsEllipseItem(parent)
    {
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override
    {
        if (change == ItemPositionChange && scene()) // 检测位置变化
        {
            QPointF newPos = value.toPointF(); // 获取新位置

            // 获取场景边界
            QRectF sceneRect = scene()->sceneRect();
            //QRectF rect = boundingRect(); // 获取当前项的边界矩形

            // 限制新位置在场景边界内
            qreal left = sceneRect.left();
            qreal right = sceneRect.right(); //- rect.width();
            qreal top = sceneRect.top();
            qreal bottom = sceneRect.bottom();// - rect.height()*2;

            newPos.setX(qMax(left, qMin(newPos.x(), right))); // 限制 X 范围
            newPos.setY(qMax(top, qMin(newPos.y(), bottom))); // 限制 Y 范围

            return newPos; // 返回修正后的位置
        }
        return QGraphicsEllipseItem::itemChange(change, value);
    }
};



class LQGraphicsRectItem :public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    LQGraphicsRectItem(QGraphicsItem *parent = nullptr);
    void setRect_LQ(QRectF newRect);
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;


private:
    QVector<LQGraphicsEllipseItem*> handles;

};




class LQGraphicsTextItem : public QGraphicsTextItem
{
public:
    LQGraphicsTextItem(QGraphicsItem *parent = nullptr)
        : QGraphicsTextItem(parent)
    {
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override
    {


        if (change == ItemPositionHasChanged || change == ItemTransformHasChanged)
        {
            setPos(this->parentItem()->pos());

        }
        else if (change == ItemSceneHasChanged)
        {
        }
        else if (change == ItemTransformHasChanged)
        {
        }

        return QGraphicsTextItem::itemChange(change, value);
    }
};




class LQGraphicsPathItem : public QGraphicsPathItem
{
public:
    LQGraphicsPathItem(QGraphicsItem *parent = nullptr);

    void startPathPoint(QPointF point);
    void appendPathPoint(QPointF point);
    void insertPathPoint(int index,QPointF point);
    void deletePathPoint(LQGraphicsEllipseItem *eleItem);
    void endPathPoint();
    bool isCloseFirstPathPoint(QPointF point);
    int pathPointSize();

    void insertEllPoint(QPointF point,int index=-1);
    int findSegmentAt(const QPointF &point);

    QList<LQGraphicsEllipseItem *>  childEllipsePoints(){return m_allPointsEll;}
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    // 重写sceneEventFilter来过滤子项事件
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

    // 计算点到线段的最短距离
    qreal distanceToSegment(const QPointF &point, const QLineF &segment);
    void updateItem();
private:
    QList<LQGraphicsEllipseItem *> m_allPointsEll;
    bool m_bhflag = false;
     QGraphicsTextItem *currentText=nullptr;
};























#endif // LQGRAPHICSITEM_H
