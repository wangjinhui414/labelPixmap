#ifndef LQGRAPHICSSCENE_H
#define LQGRAPHICSSCENE_H

#include <QObject>
#include <QWidget>
#include <QGraphicsScene>
#include "global_data.h"
#include "graphic/lqgraphicsitem.h"

class LQGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit LQGraphicsScene(QObject *parent = nullptr);
     ~LQGraphicsScene();

    void clearLashInfos();
    void loadPiamap(QString filePath);
    void setScenFill();
    void rectPloygon_item_delete();
    void rect_item_roate(qreal arg);
    void rect_item_widthAddPlus(qreal var);
    void rect_item_heightAddPlus(qreal var);
    void rect_item_change();
    void rect_item_copy();
    void rect_item_paste();

    void rect_ItemClassChange(LQGraphicsRectItem *rectItem,bool flag=true);
    void rect_mousePressEvent(QGraphicsSceneMouseEvent* event) ;
    void rect_mouseMoveEvent(QGraphicsSceneMouseEvent* event) ;
    void rect_mouseReleaseEvent(QGraphicsSceneMouseEvent* event) ;
    void rect_keyPressEvent(QKeyEvent *event);
    void rect_keyReleaseEvent(QKeyEvent *event);
    void rect_drawForeground(QPainter *painter, const QRectF &rect);
    void rect_mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void polygon_mousePressEvent(QGraphicsSceneMouseEvent* event) ;
    void polygon_mouseMoveEvent(QGraphicsSceneMouseEvent* event) ;
    void polygon_mouseReleaseEvent(QGraphicsSceneMouseEvent* event) ;
    void polygon_keyPressEvent(QKeyEvent *event);
    void polygon_keyReleaseEvent(QKeyEvent *event);
    void polygon_drawForeground(QPainter *painter, const QRectF &rect);
    void polygon_mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void polygon_item_insertPoint();
    void polygon_item_change();
    void polygon_ItemClassChange(LQGraphicsPathItem *pathItem,bool flag=true);
    void updatePath();
    QString showDialogAtMousePos();

signals:
    void signal_sendPosition(QPointF pf);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override ;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override ;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override ;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

public slots:

private:
    QPointF m_curPos;
    //矩形标注相关
    LQGraphicsRectItem* selectedItem = nullptr;
    QPointF m_rectSPos;
    bool m_rectDrawing = false;
    LQGraphicsRectItem* m_rectItem = nullptr;
    bool m_altFlag = false;
    QMap<LQGraphicsRectItem*,QPointF> m_sSizes;//记录Alt开始的矩形大小
    QMap<LQGraphicsRectItem*,QPointF> m_sDis;//记录Alt开始的距离大小
    //多边形标注相关
    bool isDrawing=false; // 是否正在绘制
    LQGraphicsPathItem *m_polygonItem = nullptr; // 路径项

};

#endif // LQGRAPHICSSCENE_H
