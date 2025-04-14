#include "lqgraphicsscene.h"
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QImageReader>

// 计算 QPointF 到 P1、P2 直线的垂直距离
double pointToLineDistance(const QPointF& P, const QPointF& P1, const QPointF& P2) {
    double A = P2.y() - P1.y();
    double B = P1.x() - P2.x();
    double C = P2.x() * P1.y() - P1.x() * P2.y();

    double numerator = qAbs(A * P.x() + B * P.y() + C);
    double denominator = qSqrt(A * A + B * B);

    return denominator == 0 ? 0 : numerator / denominator;
}


LQGraphicsScene::LQGraphicsScene(QObject *parent): QGraphicsScene(parent)
{


}

LQGraphicsScene::~LQGraphicsScene()
{

}

void LQGraphicsScene::clearLashInfos()
{
    //矩形标注相关
    selectedItem = nullptr;
    m_rectDrawing = false;
    m_rectItem = nullptr;
    m_altFlag = false;
    m_sSizes.clear();
    m_sDis.clear();
    isDrawing=false; // 是否正在绘制
    m_polygonItem = nullptr; // 路径项
}

void LQGraphicsScene::loadPiamap(QString filePath)
{    
    clear(); // 清除之前的项
    clearLashInfos();
    //加载图片方向
    QImageReader reader(filePath);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull())return ;

    QPixmap pixmap = QPixmap::fromImage(image);


    QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(pixmap);
    addItem(pixmapItem);

    pixmapItem->setZValue(-1);
    pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable,false);
    // 设置场景大小为图片的大小
    setSceneRect(pixmapItem->boundingRect());

    setScenFill();
}

void LQGraphicsScene::setScenFill()
{
    auto sview = this->views().at(0);
    sview->setRenderHint(QPainter::Antialiasing);

    // 获取视图的尺寸
    QSize viewSize = sview->size();

    // 计算目标尺寸（视图的 90%）
    QSizeF targetSize(viewSize.width() * 0.95, viewSize.height() * 0.95);

    // 获取场景的边界矩形
    QRectF sceneRect = this->sceneRect();

    // 计算缩放比例
    qreal xScale = targetSize.width() / sceneRect.width();
    qreal yScale = targetSize.height() / sceneRect.height();
    qreal scale = qMin(xScale, yScale);

    // 应用缩放
    sview->resetTransform(); // 重置变换
    sview->scale(scale, scale);

    // 将场景居中
    sview->centerOn(sceneRect.center());
}

void LQGraphicsScene::rectPloygon_item_delete()
{
    QList<QGraphicsItem *> sitems = selectedItems();
    foreach(auto one,sitems)
    {
        if(one->type() == LQGraphicsRectItem::Type || one->type() == LQGraphicsPathItem::Type)
            removeItem(one);
        if(one->type() == LQGraphicsEllipseItem::Type)//删除的是源点
        {
            LQGraphicsPathItem *pathItem = static_cast<LQGraphicsPathItem*>(one->parentItem());
            pathItem->deletePathPoint(static_cast<LQGraphicsEllipseItem*>(one));
            removeItem(one);
        }
    }
}

void LQGraphicsScene::rect_item_roate(qreal arg)
{
    QList<QGraphicsItem *> sitems = selectedItems();
    for(int i=0;i!=sitems.size();i++)
    {
        if(sitems[i]->type() == LQGraphicsRectItem::Type)
            sitems[i]->setRotation(sitems[i]->rotation() + arg);
    }

}

void LQGraphicsScene::rect_item_widthAddPlus(qreal var)
{
    QList<QGraphicsItem *> sitems = selectedItems();
    for(int i=0;i!=sitems.size();i++)
    {
        if(sitems[i]->type() ==LQGraphicsRectItem::Type)
        {
            LQGraphicsRectItem *rectItem = static_cast<LQGraphicsRectItem*>(sitems[i]);
            QRectF rectf = rectItem->rect();
            rectItem->setRect_LQ( 
                        QRectF(rectf.x(),rectf.y(),rectf.width()+var,rectf.height())
                        );
        }
    }
}

void LQGraphicsScene::rect_item_heightAddPlus(qreal var)
{
    QList<QGraphicsItem *> sitems = selectedItems();
    for(int i=0;i!=sitems.size();i++)
    {
        if(sitems[i]->type() ==LQGraphicsRectItem::Type)
        {
            LQGraphicsRectItem *rectItem = static_cast<LQGraphicsRectItem*>(sitems[i]);
            QRectF rectf = rectItem->rect();
            rectItem->setRect_LQ(
                        QRectF(rectf.x(),rectf.y(),rectf.width(),rectf.height()+var)
                        );
        }
    }
}

void LQGraphicsScene::rect_item_change()
{
    QList<QGraphicsItem *> sitems = selectedItems();
    for(int i=0;i!=sitems.size();i++)
    {
        if(sitems[i]->type() ==LQGraphicsRectItem::Type)
        {
            LQGraphicsRectItem *rectItem = static_cast<LQGraphicsRectItem*>(sitems[i]);
            rect_ItemClassChange(rectItem);
            //break;
        }
    }
}

void LQGraphicsScene::rect_item_copy()
{
    // 在这里添加处理 Ctrl+C 的逻辑
    // 例如：复制选中的图元
    auto Items = selectedItems();
    foreach(auto one,Items){
        if(one->type() == LQGraphicsRectItem::Type)
            selectedItem = static_cast<LQGraphicsRectItem*>(one);
    }
}

void LQGraphicsScene::rect_item_paste()
{
    if (selectedItem!=nullptr)
    {
        QPointF pf =  m_curPos;
        QRectF rect1 = selectedItem->rect();
        qreal arg1 = selectedItem->rotation();
        golbalState.curIndex = selectedItem->data(Qt::UserRole+8).toInt();
        LQGraphicsRectItem *newItem = new LQGraphicsRectItem();
        addItem(newItem);
        newItem->setRect(QRectF(0,0,rect1.width(),rect1.height()));
        newItem->setRotation(arg1);
        newItem->setPos(pf);
    }
}

QString LQGraphicsScene::showDialogAtMousePos()
{
    // 获取当前鼠标的全局位置
    QPoint mousePos = QCursor::pos();

    // 创建输入对话框
    QInputDialog dialog(nullptr);
    dialog.setWindowTitle(tr("选择项目"));
    dialog.setLabelText(tr("请选择一个选项:"));
    //dialog.setOption(QInputDialog::UseListViewForComboBoxItems, true); // 可选：使用列表视图
    dialog.setComboBoxItems(golbalState.classNames.toList());

    // 获取内部ComboBox
    QComboBox *combo = dialog.findChild<QComboBox*>();
    if (combo) {
        combo->setCurrentIndex(golbalState.curCsIndex());
    }


    // 调整对话框大小（可选）
    dialog.resize(300, 150);

    // 将对话框移动到鼠标位置
    dialog.move(QPoint(mousePos.x()-50,mousePos.y()-80));

    // 显示对话框
    if (dialog.exec() == QDialog::Accepted) {
        QString selected = dialog.textValue();
        return selected;
    }
    return golbalState.curCsName();
}

void LQGraphicsScene::rect_ItemClassChange(LQGraphicsRectItem *rectItem,bool flag)
{
    if(flag)
    {
        QString typestr = showDialogAtMousePos();//QInputDialog::getItem(nullptr,"类别选择","请选择标注类别！",golbalState.classNames.toList(),golbalState.curCsIndex(),false);
        golbalState.curIndex = golbalState.classNames.indexOf(typestr);
    }
    //边框颜色
    rectItem->setData(Qt::UserRole+8,golbalState.curCsIndex());
    rectItem->setPen(QPen(golbalState.fontColor, golbalState.lineWidth));
    if(!golbalState.classIshow[golbalState.curCsIndex()]) rectItem->setVisible(false);
    else rectItem->setVisible(true);
    //蒙版颜色
    QColor color = golbalState.curCsColor();
    color.setAlpha(golbalState.alpha);
    QBrush brush(color);
    rectItem->setBrush(brush);
    //rectItem->setSelected(true);
    //字体颜色
    QGraphicsTextItem *rectText = nullptr;
    auto childs = rectItem->childItems();
    for(auto one:childs){
        if(one->type() == QGraphicsTextItem::Type)
            rectText = static_cast<QGraphicsTextItem*>(one);
    }
    QFont font;
    font.setPointSize(golbalState.fontSize);
    rectText->setFont(font);
    rectText->setPlainText(golbalState.curCsName());
    rectText->setDefaultTextColor(golbalState.fontColor);

}

void LQGraphicsScene::rect_mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        m_rectSPos = event->scenePos();
        m_rectItem = nullptr;
        m_rectDrawing = true;
    }
}

void LQGraphicsScene::rect_mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_rectDrawing) {
        if(m_rectItem==nullptr){
            m_rectItem =  new LQGraphicsRectItem();
            addItem(m_rectItem);

        }
        //动态更新矩形
        QPointF endPos = event->scenePos();
        qreal width = qAbs(endPos.x() - m_rectSPos.x());
        qreal height = qAbs(endPos.y() - m_rectSPos.y());
        QPointF topLeft = QPointF(qMin(m_rectSPos.x(), endPos.x()), qMin(m_rectSPos.y(), endPos.y()));
        m_rectItem->setRect_LQ(QRectF(QPointF(0,0), QSizeF(width, height)));
        m_rectItem->setTransformOriginPoint(QPointF(0,0));
        m_rectItem->setPos(topLeft);
    }
    //快速大小调整
    if(m_altFlag)
    {
        QList<QGraphicsItem *> sitems = this->selectedItems();
        for(int i=0;i!=sitems.size();i++)
        {
            if(sitems[i]->type() ==LQGraphicsRectItem::Type)
            {
                LQGraphicsRectItem *rectItem = static_cast<LQGraphicsRectItem*>(sitems[i]);
                QPointF p0 = rectItem->rect().topLeft();
                QPointF p1 = rectItem->rect().topRight();
                QPointF p2 = rectItem->rect().bottomLeft();
                p0 = rectItem->mapToScene(p0);
                p1 = rectItem->mapToScene(p1);
                p2 = rectItem->mapToScene(p2);

                QRectF rectf = rectItem->rect();

                QPointF sSize = m_sSizes[rectItem];
                //计算Alt点离上线的距离
                qreal dis1 = pointToLineDistance(event->scenePos(),p0,p2);
                qreal dis2 = pointToLineDistance(event->scenePos(),p0,p1);

                //计算距离增量
                qreal dltDisX = dis1-m_sDis[rectItem].x();
                qreal dltDisY = dis2-m_sDis[rectItem].y();
                //设置新的宽度和高度
                rectf.setWidth(sSize.x() + dltDisX);
                rectf.setHeight(sSize.y() + dltDisY);
                rectItem->setRect_LQ(rectf);
            }
        }

    }

}

void LQGraphicsScene::rect_mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton && m_rectDrawing) {
        //类别注释
        if(m_rectItem)
        {
            rect_ItemClassChange(m_rectItem);           
        }
        m_rectDrawing = false;
        m_rectItem = nullptr;
    }
}

void LQGraphicsScene::rect_keyPressEvent(QKeyEvent *event)
{
    //快速大小调整
    if (event->key() == Qt::Key_Alt) {
        m_altFlag = true;
        //记录下的位置
        // 获取视图
        QGraphicsView *view = views().first(); // 假设只有一个视图
        QPointF m_Spos;
        if (view) {
            // 将全局坐标转换为视图中的坐标
            QPoint viewPos = view->mapFromGlobal(QCursor::pos());
            // 将视图中的坐标转换为场景中的坐标
            m_Spos = view->mapToScene(viewPos);
        }

        QList<QGraphicsItem *> sitems = this->selectedItems();
        for(int i=0;i!=sitems.size();i++)
        {
            if(sitems[i]->type() == LQGraphicsRectItem::Type)
            {
                LQGraphicsRectItem *rectItem = static_cast<LQGraphicsRectItem*>(sitems[i]);
                QPointF p0 = rectItem->rect().topLeft();
                QPointF p1 = rectItem->rect().topRight();
                QPointF p2 = rectItem->rect().bottomLeft();
                p0 = rectItem->mapToScene(p0);
                p1 = rectItem->mapToScene(p1);
                p2 = rectItem->mapToScene(p2);
                //计算Alt点离上线的距离;
                qreal dis1 = pointToLineDistance(m_Spos,p0,p2);
                //计算Alt点离左线的距离
                qreal dis2 = pointToLineDistance(m_Spos,p0,p1);
               //记录开始距离差
                m_sDis[rectItem] = QPointF(dis1,dis2);
                m_sSizes[rectItem] = QPointF(rectItem->rect().width(),rectItem->rect().height());
            }
        }
    }

    // 检查是否按下了 Ctrl + C

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C)
    {
        rect_item_copy();
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_V)
    {
        rect_item_paste();
    }

}

void LQGraphicsScene::rect_keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt) {
        m_altFlag = false;
        m_sSizes.clear();
        m_sDis.clear();
    }
}

void LQGraphicsScene::rect_drawForeground(QPainter *painter, const QRectF &rect)
{
Q_UNUSED(painter)
    Q_UNUSED(rect)
}

void LQGraphicsScene::rect_mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QList<QGraphicsItem *>  tmpitems = this->items(event->scenePos());

    foreach(auto one,tmpitems)
    {
        if(one->type() == LQGraphicsRectItem::Type)
        {

            rect_ItemClassChange(static_cast<LQGraphicsRectItem*>(one));
        }
    }
}

void LQGraphicsScene::polygon_mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    if (event->button() == Qt::RightButton)
    {
        QPointF currentPoint = event->scenePos();
        if (!isDrawing)
        {
           //创建路径Item
           m_polygonItem = new LQGraphicsPathItem;
           addItem(m_polygonItem);
           m_polygonItem->startPathPoint(currentPoint);// 添加起始点
           isDrawing = true;
        }
        else
        {
           // 检查是否闭合路径
           if (m_polygonItem->isCloseFirstPathPoint(currentPoint))
           {
               // 闭合路径
               m_polygonItem->endPathPoint();
               isDrawing = false;
               polygon_ItemClassChange(m_polygonItem);
           }
           //追加折点
           else
           {
              m_polygonItem->appendPathPoint(currentPoint);
           }
        }
    }
}


void LQGraphicsScene::updatePath()
{

}

void LQGraphicsScene::polygon_mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
}

void LQGraphicsScene::polygon_mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
}

void LQGraphicsScene::polygon_keyPressEvent(QKeyEvent *event)
{
Q_UNUSED(event)
}

void LQGraphicsScene::polygon_keyReleaseEvent(QKeyEvent *event)
{
Q_UNUSED(event)
}

void LQGraphicsScene::polygon_drawForeground(QPainter *painter, const QRectF &rect)
{
Q_UNUSED(painter)
    Q_UNUSED(rect)
}

void LQGraphicsScene::polygon_mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QList<QGraphicsItem *>  tmpitems = this->items(event->scenePos());
    foreach(auto one,tmpitems)
    {
        if(one->type() == LQGraphicsPathItem::Type)
        {
            polygon_ItemClassChange(static_cast<LQGraphicsPathItem*>(one));
        }
    }
}

void LQGraphicsScene::polygon_item_insertPoint()
{
    for(auto one : items()){
        if(one->type() == LQGraphicsPathItem::Type){
            LQGraphicsPathItem *pathItem =  static_cast<LQGraphicsPathItem*>(one);
            int iPos = pathItem->findSegmentAt(m_curPos);
            if(iPos>=0)
                pathItem->insertPathPoint(iPos+1,m_curPos);
        }
    }
}

void LQGraphicsScene::polygon_item_change()
{
    QList<QGraphicsItem *> sitems = selectedItems();
    for(int i=0;i!=sitems.size();i++)
    {
        if(sitems[i]->type() ==LQGraphicsPathItem::Type)
        {
            LQGraphicsPathItem *pathItem = static_cast<LQGraphicsPathItem*>(sitems[i]);
            polygon_ItemClassChange(pathItem);
            //break;
        }
    }
}

void LQGraphicsScene::polygon_ItemClassChange(LQGraphicsPathItem *pathItem, bool flag)
{
    if(flag)
    {
        QString typestr = showDialogAtMousePos();
        golbalState.curIndex = golbalState.classNames.indexOf(typestr);
    }
    //边框颜色
    pathItem->setData(Qt::UserRole+8,golbalState.curCsIndex());
    pathItem->setPen(QPen(golbalState.fontColor, golbalState.lineWidth));
    if(!golbalState.classIshow[golbalState.curCsIndex()]) pathItem->setVisible(false);
    else pathItem->setVisible(true);
    //蒙版颜色
    QColor color = golbalState.curCsColor();
    color.setAlpha(golbalState.alpha);
    QBrush brush(color);
    pathItem->setBrush(brush);
    //rectItem->setSelected(true);
    //字体颜色
    QGraphicsTextItem *rectText = nullptr;
    auto childs = pathItem->childItems();
    for(auto one:childs){
        if(one->type() == QGraphicsTextItem::Type)
            rectText = static_cast<QGraphicsTextItem*>(one);
    }
    QFont font;
    font.setPointSize(golbalState.fontSize);
    rectText->setFont(font);
    rectText->setPlainText(golbalState.curCsName());
    rectText->setDefaultTextColor(golbalState.fontColor);
}



void LQGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(golbalState.labelMode == LABEL_RECT_MODE)
        rect_mousePressEvent(event);
    else if(golbalState.labelMode == LABEL_POLYGON_MODE)
        polygon_mousePressEvent(event);
    QGraphicsScene::mousePressEvent(event);
}

void LQGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_curPos = event->scenePos();
    emit signal_sendPosition(m_curPos);
    if(golbalState.labelMode == LABEL_RECT_MODE)
        rect_mouseMoveEvent(event);
    else if(golbalState.labelMode == LABEL_POLYGON_MODE)
        polygon_mouseMoveEvent(event);

    QGraphicsScene::mouseMoveEvent(event);
}

void LQGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(golbalState.labelMode == LABEL_RECT_MODE)
        rect_mouseReleaseEvent(event);
    else if(golbalState.labelMode == LABEL_POLYGON_MODE)
        polygon_mouseReleaseEvent(event);
    QGraphicsScene::mouseReleaseEvent(event);
}

void LQGraphicsScene::keyPressEvent(QKeyEvent *event)
{
    if(golbalState.labelMode == LABEL_RECT_MODE)
        rect_keyPressEvent(event);
    else if(golbalState.labelMode == LABEL_POLYGON_MODE)
        polygon_keyPressEvent(event);
    QGraphicsScene::keyPressEvent(event);
}

void LQGraphicsScene::keyReleaseEvent(QKeyEvent *event)
{
    if(golbalState.labelMode == LABEL_RECT_MODE)
        rect_keyReleaseEvent(event);
    else if(golbalState.labelMode == LABEL_POLYGON_MODE)
        polygon_keyReleaseEvent(event);
    QGraphicsScene::keyReleaseEvent(event);
}

void LQGraphicsScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if(golbalState.labelMode == LABEL_RECT_MODE)
        rect_drawForeground(painter,rect);
    else if(golbalState.labelMode == LABEL_POLYGON_MODE)
        polygon_drawForeground(painter,rect);
    QGraphicsScene::drawForeground(painter,rect);
}

void LQGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QList<QGraphicsItem *>  tmpitems = this->items(event->scenePos());

    foreach(auto one,tmpitems)
    {
        if(one->type() == LQGraphicsRectItem::Type)
        {

            rect_ItemClassChange(static_cast<LQGraphicsRectItem*>(one));
        }
        else if(one->type() == LQGraphicsPathItem::Type)
        {
            polygon_ItemClassChange(static_cast<LQGraphicsPathItem*>(one));
        }
    }
    QGraphicsScene::mouseDoubleClickEvent(event);
}

