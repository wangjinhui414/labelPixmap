#include "lqlistview.h"
#include <QDir>
#include "lqtreeview.h"




LQListView::LQListView(QWidget *parent):QListView(parent)
{
    m_listModel = new QStandardItemModel;
    this->setModel(m_listModel); // 设置模型
    setItemDelegate(new ImageTextDelegate(this));
    // 配置布局：水平流式布局 + 自动换行
    setHorizontalFlowMode();
    //this->setViewMode(QListView::IconMode); // 设置视图模式为缩略图
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());
    // 设置多选模式
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    // 获取 QListView 的 selectionModel
    QItemSelectionModel *selectionModel = this->selectionModel();
    // 连接 currentChanged 信号到槽函数
    QObject::connect(selectionModel, &QItemSelectionModel::currentChanged, [this](const QModelIndex &current, const QModelIndex &previous) {
        Q_UNUSED(previous)
        if (current.isValid()) {
            //qDebug()<<current.data(Qt::UserRole+1).toString();
            emit signal_itemSelectChange(current.data(Qt::UserRole+1).toString());
        } else {

        }

    });
}

LQListView::~LQListView()
{

}

void LQListView::slot_loadImagesFromDirectory(QString dirPath)
{
     QString sessionId = QUuid::createUuid().toString();

     // 清空过期会话（避免内存泄漏）
     clearExpiredSessions();

     // 初始化新会话
     QMutexLocker locker(&m_sessionMutex);
     LoadSession &session = m_activeSessions[sessionId];
     session.dirPath = dirPath;

     // 获取目录下的所有图片文件
     QDir dir(dirPath);
     QStringList imageFiles = dir.entryList({"*.jpg", "*.png", "*.bmp", "*.jpeg"}, QDir::Files);
     // 清空旧数据并预填充占位项
     m_listModel->clear();
     m_listModel->setRowCount(imageFiles.size()); // 设置行数
     session.files = QSet<QString>(imageFiles.begin(), imageFiles.end());

     // 提交异步加载任务
     //for (const QString &file : files) {
     for (int i = 0; i < imageFiles.size(); ++i) {
         QString file = imageFiles[i];
         QString filePath = dir.filePath(file);
         ImageLoadTask *task = new ImageLoadTask(i,filePath, sessionId);
         connect(task, &ImageLoadTask::loadCompleted, this, &LQListView::slot_onImageLoaded);
         QThreadPool::globalInstance()->start(task);
     }

     // 清空当前列表
     m_listModel->clear();
}

void LQListView::slot_onImageLoaded(int index,const QString &sessionId, const QString &filePath, const QPixmap &pixmap) {
    QMutexLocker locker(&m_sessionMutex);

    // 检查会话是否有效（用户可能已切换到其他目录）
    if (!m_activeSessions.contains(sessionId)) {
        qDebug() << "忽略过期会话的图片:" << filePath;
        return;
    }

    // 检查文件是否属于当前会话（防止旧任务干扰）
    LoadSession &session = m_activeSessions[sessionId];
    if (!session.files.contains(QFileInfo(filePath).fileName())) {
        qDebug() << "忽略不属于会话的文件:" << filePath;
        return;
    }

    // 添加项到列表视图
    QStandardItem *item = new QStandardItem;
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setData(pixmap, Qt::DecorationRole);
    item->setData(QFileInfo(filePath).fileName(), Qt::DisplayRole);
    item->setData(filePath, Qt::UserRole+1);// 完整路径保存到 UserRole
    //listModel->appendRow(item);
    // 按索引插入到模型
    m_listModel->setItem(index, 0, item); // 假设使用单列模型
}



void LQListView::clearExpiredSessions() {
    QMutexLocker locker(&m_sessionMutex);
    QMap<QString, LoadSession>::iterator it = m_activeSessions.begin();
    while (it != m_activeSessions.end()) {
        // 保留最近5个会话（根据需求调整）
        if (m_activeSessions.size() > 5) {
            it = m_activeSessions.erase(it);
        } else {
            ++it;
        }
    }
}

void LQListView::setHorizontalFlowMode()
{
    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setSpacing(5); // 可选：调整项间距
    updateGeometry(); // 更新布局
}

void LQListView::setVerticalListMode()
{
    setFlow(QListView::TopToBottom); // 默认垂直流
    setWrapping(false); // 关闭自动换行
    setResizeMode(QListView::Fixed); // 或 Adjust，根据需求
    setSpacing(2); // 可选：恢复默认间距
    updateGeometry(); // 更新布局
}

void LQListView::reloadOneImage(int row, QString filePathName)
{
    QStandardItem *item = m_listModel->item(row,0);
    if(item==nullptr) return;
    QImage img(filePathName);
    if (!img.isNull()) {
        img = img.scaled(DITEM_WIDTH, DITEM_HEIGHT);
        QPixmap pixmap = QPixmap::fromImage(img);
        item->setData(pixmap, Qt::DecorationRole);
    }
}


