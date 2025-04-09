#include "lqgraphicsitem.h"



LQGraphicsPathItem::LQGraphicsPathItem(QGraphicsItem *parent) : QGraphicsPathItem(parent)
{
    //创建路径Item
    setFlag(QGraphicsItem::ItemIsSelectable);
    //setFlag(QGraphicsItem::ItemIsMovable);
    // 启用子项事件过滤
     setFiltersChildEvents(true);


}

void LQGraphicsPathItem::startPathPoint(QPointF point)
{
    //创建标签
    currentText = new QGraphicsTextItem(golbalState.curCsName());
    QFont font;
    font.setPointSize(golbalState.fontSize);
    currentText->setFont(font);
    currentText->setDefaultTextColor(golbalState.fontColor);
    currentText->setFlag(QGraphicsItem::ItemIsSelectable);
    currentText->setFlag(QGraphicsItem::ItemIsMovable);
    currentText->setParentItem(this);
    currentText->setPos(point);
    appendPathPoint(point);

    // 设置画刷颜色和透明度
    QColor color = golbalState.curCsColor();
    color.setAlpha(golbalState.alpha);
    QBrush brush(color);
    setBrush(brush);
}




void LQGraphicsPathItem::appendPathPoint(QPointF point)
{
    insertEllPoint(point);

}

void LQGraphicsPathItem::insertPathPoint(int index, QPointF point)
{
    insertEllPoint(point,index);
}

void LQGraphicsPathItem::deletePathPoint(LQGraphicsEllipseItem *eleItem)
{
    m_allPointsEll.removeOne(eleItem);
    updateItem();
}


void LQGraphicsPathItem::endPathPoint()
{
    m_bhflag = true;
    updateItem();
}

bool LQGraphicsPathItem::isCloseFirstPathPoint(QPointF point)
{
    if (m_allPointsEll.size()<3) return false;
    qreal fg = QLineF(point, m_allPointsEll.first()->scenePos()).length();

    return fg < 10?true:false; // 阈值 10
}

int LQGraphicsPathItem::pathPointSize()
{
    return m_allPointsEll.size();
}

void LQGraphicsPathItem::insertEllPoint(QPointF point,int index)
{
    QRectF rect = scene()->sceneRect();
    QPointF ellPoint = point;
    if(ellPoint.x()<0) ellPoint.setX(0);
    if(ellPoint.y()<0) ellPoint.setY(0);
    if(ellPoint.x()>rect.width()) ellPoint.setX(rect.width());
    if(ellPoint.y()>rect.height()) ellPoint.setY(rect.height());

    LQGraphicsEllipseItem *item = new LQGraphicsEllipseItem(); // 点的大小
    item->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    int wsize = golbalState.pointSize;
    item->setRect(-wsize, -wsize, wsize*2, wsize*2);
    item->setPos(ellPoint);
    item->setPen(QPen(Qt::red, 1));
    item->setBrush(Qt::red);
    item->setParentItem(this);
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    item->setFlag(QGraphicsItem::ItemIsMovable);
    // 连接场景的 changed 信号
    if(index==-1)
        m_allPointsEll.append(item);
    else
        m_allPointsEll.insert(index,item);
    updateItem();
}

void LQGraphicsPathItem::updateItem()
{
    //更新路径
    QPainterPath curDrawpath;
    if(m_bhflag){
        setPen(QPen(Qt::green, golbalState.lineWidth));
    }
    else{
        setPen(QPen(Qt::yellow, golbalState.lineWidth));
    }
    for(int i=0;i!=m_allPointsEll.size();i++)
    {
        if(i==0)
            curDrawpath.moveTo(m_allPointsEll[i]->scenePos());
        else
            curDrawpath.lineTo(m_allPointsEll[i]->scenePos());
    }
    //路径已经闭合重新刷新时也闭合
    if(m_bhflag){
        curDrawpath.lineTo(m_allPointsEll.first()->scenePos());
        curDrawpath.closeSubpath();
    }
    setPath(curDrawpath);


}

QVariant LQGraphicsPathItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged || change == ItemTransformHasChanged)
    {
        //setPos(this->parentItem()->pos());
    }
    else if (change == ItemSceneHasChanged)
    {
    }
    else if (change == ItemTransformHasChanged)
    {
    }

    return QGraphicsPathItem::itemChange(change, value);
}

bool LQGraphicsPathItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{

    LQGraphicsEllipseItem *ellItem = static_cast<LQGraphicsEllipseItem*>(watched);
    if(m_allPointsEll.contains(ellItem))
    {

        updateItem();
    }

    return QGraphicsPathItem::sceneEventFilter(watched, event);

}

int LQGraphicsPathItem::findSegmentAt(const QPointF &point)
{
    if(m_allPointsEll.size()<2) return -1;
    for(int i=0;i!=m_allPointsEll.size();i++)
    {
        int sPos = i;
        int ePos = i==(m_allPointsEll.size()-1)?0:(i+1);
        QLineF segment(m_allPointsEll[sPos]->scenePos(), m_allPointsEll[ePos]->scenePos());
        if (distanceToSegment(point,segment) < 5) { // 阈值 10
            return i;
        }
    }
    return -1;
}

qreal LQGraphicsPathItem::distanceToSegment(const QPointF &point, const QLineF &segment)
{
    QPointF p1 = segment.p1(); // 线段起点
    QPointF p2 = segment.p2(); // 线段终点
    QPointF p = point;         // 目标点

    // 计算线段长度
    qreal segmentLength = segment.length();
    if (segmentLength == 0) {
       // 如果线段长度为 0，直接返回点到起点的距离
       return QLineF(p, p1).length();
    }

    // 计算点到线段起点的向量
    QPointF p1p = p - p1;
    // 计算线段方向的单位向量
    QPointF p1p2 = (p2 - p1) / segmentLength;

    // 计算投影长度
    qreal projection = QPointF::dotProduct(p1p, p1p2);

    if (projection < 0) {
       // 投影在线段起点之外，返回点到起点的距离
       return QLineF(p, p1).length();
    } else if (projection > segmentLength) {
       // 投影在线段终点之外，返回点到终点的距离
       return QLineF(p, p2).length();
    } else {
       // 投影在线段上，计算垂直距离
       QPointF closestPoint = p1 + p1p2 * projection;
       return QLineF(p, closestPoint).length();
    }
}

LQGraphicsRectItem::LQGraphicsRectItem(QGraphicsItem *parent):QObject(nullptr), QGraphicsRectItem(parent)  // 显式初始化QObject
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);  // 必须启用此项
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);

    setData(Qt::UserRole+8,golbalState.curCsIndex());
    setPen(QPen(golbalState.fontColor, golbalState.lineWidth));  // 只设置边框颜色，不填充
    //蒙版颜色
    QColor color = golbalState.curCsColor();
    color.setAlpha(golbalState.alpha);
    QBrush brush(color);
    setBrush(brush);
    setSelected(true);
    //创建类别注释
    QGraphicsTextItem *rectText = new QGraphicsTextItem(golbalState.classNames[golbalState.curIndex]);
    QFont font;
    font.setPointSize(golbalState.fontSize);
    rectText->setFont(font);
    rectText->setDefaultTextColor(golbalState.fontColor);
    rectText->setFlag(QGraphicsItem::ItemIsSelectable);
    rectText->setFlag(QGraphicsItem::ItemIsMovable);
    rectText->setParentItem(this);
    //this->setFiltersChildEvents()
}

void LQGraphicsRectItem::setRect_LQ(QRectF newRect)
{
   //        // 获取场景范围
   //        QRectF sceneRect = scene() ? scene()->sceneRect() : QRectF();
   //        if (!sceneRect.isNull()) {
   //            // 计算当前矩形的 4 个顶点（考虑旋转）
   //            QTransform transform = this->transform();
   //            QPolygonF polygon = transform.mapToPolygon(newRect.toRect());

   //            // 检查所有顶点是否在场景范围内
   //            for (const QPointF &point : polygon) {
   //                //qDebug()<<point;
   //                if (!sceneRect.contains(point)) {

   //                    return;  // 超出范围，不更新
   //                }
   //            }
   //        }
    QRectF oldRect = rect();
    QGraphicsRectItem::setRect(newRect);  // 在范围内，更新大小
    QRectF sceneRect = scene() ? scene()->sceneRect() : QRectF();
    QRectF sceneBoundingRect = this->sceneBoundingRect();
    if(!sceneRect.contains(sceneBoundingRect)){
        QGraphicsRectItem::setRect(oldRect);
    }

}

//QSizeF calculateExceededSize(LQGraphicsRectItem *rectItem, QGraphicsScene *scene)
//{
//    QRectF sceneRect = scene->sceneRect();
//    QRectF itemRect = rectItem->sceneBoundingRect();



//    return QSizeF(exceededWidth, exceededHeight);
//}

QVariant LQGraphicsRectItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
//    if (change == ItemPositionHasChanged || change == ItemTransformHasChanged )
//    {
//        // 获取矩形的新的位置
//        QPointF newPos = value.toPointF();
//        QRectF rect = this->rect();

//        // 获取场景的边界 (通过 scene() 获取场景对象)
//        QRectF sceneRect = scene()->sceneRect();

//        // 获取矩形的旋转角度
//        qreal angle = rotation();

//        // 获取矩形的左上角 (旋转点)
//        QPointF topLeft = rect.topLeft();

//        // 计算旋转矩阵
//        QTransform transform;
//        transform.translate(topLeft.x(), topLeft.y());  // 将旋转点移到左上角
//        transform.rotate(angle);  // 旋转
//        transform.translate(-topLeft.x(), -topLeft.y());  // 恢复原位置

//        // 计算旋转后的矩形的外接矩形
//        QRectF rotatedRect = transform.mapRect(rect);

//        // 限制矩形不能移出场景，确保旋转后的矩形的外接矩形在场景内
//        qreal newX = newPos.x();
//        qreal newY = newPos.y();

//        if (newX < sceneRect.left() - rotatedRect.left()) {
//            newX = sceneRect.left() - rotatedRect.left();
//        } else if (newX + rotatedRect.width() > sceneRect.right()) {
//            newX = sceneRect.right() - rotatedRect.width();
//        }

//        if (newY < sceneRect.top() - rotatedRect.top()) {
//            newY = sceneRect.top() - rotatedRect.top();
//        } else if (newY + rotatedRect.height() > sceneRect.bottom()) {
//            newY = sceneRect.bottom() - rotatedRect.height();
//        }

//        // 手动更新物品的位置
//        setPos(QPointF(newX, newY));

//        // 返回调整后的新位置
//        return QPointF(newX, newY);
//    }
    if (change == ItemPositionHasChanged || change == ItemTransformHasChanged )
    {
        QRectF sceneRect = scene() ? scene()->sceneRect() : QRectF();
        QRectF itemBoundingRect = this->sceneBoundingRect();

        // 计算超出场景的宽度（左右两侧）
        qreal exceededWidth = 0;
        if (itemBoundingRect.left() < sceneRect.left()) {
            exceededWidth = itemBoundingRect.left() - sceneRect.left();
        }
        if (itemBoundingRect.right() > sceneRect.right()) {
            exceededWidth = itemBoundingRect.right() - sceneRect.right();
        }

        // 计算超出场景的高度（上下两侧）
        qreal exceededHeight = 0;
        if (itemBoundingRect.top() < sceneRect.top()) {
            exceededHeight = itemBoundingRect.top() - sceneRect.top();
        }
        if (itemBoundingRect.bottom() > sceneRect.bottom()) {
            exceededHeight = itemBoundingRect.bottom() - sceneRect.bottom();
        }
        QPointF newPos = value.toPointF();
        newPos.setX(newPos.x()-exceededWidth);
        newPos.setY(newPos.y()-exceededHeight);
        setPos(newPos);
        return newPos;
    }
    else if (change == ItemRotationHasChanged)
    {
        QRectF sceneRect = scene() ? scene()->sceneRect() : QRectF();
        QRectF itemBoundingRect = this->sceneBoundingRect();

        // 计算超出场景的宽度（左右两侧）
        qreal exceededWidth = 0;
        if (itemBoundingRect.left() < sceneRect.left()) {
            exceededWidth = itemBoundingRect.left() - sceneRect.left();
        }
        if (itemBoundingRect.right() > sceneRect.right()) {
            exceededWidth = itemBoundingRect.right() - sceneRect.right();
        }

        // 计算超出场景的高度（上下两侧）
        qreal exceededHeight = 0;
        if (itemBoundingRect.top() < sceneRect.top()) {
            exceededHeight = itemBoundingRect.top() - sceneRect.top();
        }
        if (itemBoundingRect.bottom() > sceneRect.bottom()) {
            exceededHeight = itemBoundingRect.bottom() - sceneRect.bottom();
        }
        QPointF newPos = this->pos();
        newPos.setX(newPos.x()-exceededWidth);
        newPos.setY(newPos.y()-exceededHeight);
        //qDebug()<<newPos<<exceededWidth<<exceededHeight;
        setPos(newPos);
        return newPos;
    }
    else if (change == ItemSceneHasChanged)
    {

    }
    else if (change == ItemTransformHasChanged)
    {

    }
    return QGraphicsRectItem::itemChange(change, value);
}

