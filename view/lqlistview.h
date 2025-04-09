#ifndef LQLISTVIEW_H
#define LQLISTVIEW_H

#include <QObject>
#include <QListView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QProgressDialog>
#include <QMutex>
#include <QCache>
#include <QThreadPool>
#include <QRunnable>
#include <QtConcurrent>

#define DITEM_WIDTH  200
#define DITEM_HEIGHT 150

struct LoadSession {
    QString dirPath;      // 当前加载的目录路径
    QSet<QString> files;  // 当前目录下需要加载的文件集合
};

// 异步加载任务类
class ImageLoadTask : public QObject, public QRunnable {
    Q_OBJECT
public:
    ImageLoadTask(int index, const QString &filePath,QString sessionId)
        : m_filePath(filePath), m_sessionId(sessionId),m_fileIndex(index){}

    void run() {
        // 加载图片并缩放
        QImage img(m_filePath);
        if (!img.isNull()) {
            img = img.scaled(DITEM_WIDTH, DITEM_HEIGHT);
            QPixmap pixmap = QPixmap::fromImage(img);
            Q_EMIT loadCompleted(m_fileIndex,m_sessionId, m_filePath, QPixmap::fromImage(img));
        }
    }

signals:
    void loadCompleted(int index, QString m_sessionId, QString path, QPixmap pixmap);
private:
    QString m_filePath;
    QString m_sessionId;
    int m_fileIndex;
};

class ImageTextDelegate : public QStyledItemDelegate {
public:
    explicit ImageTextDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // 1. 保存 painter 状态
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        // 2. 绘制默认背景和焦点框
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index); // 初始化样式选项（包括选中状态）
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        // 3. 绘制选中状态的高亮边框
        if (opt.state & QStyle::State_Selected) {
            painter->setPen(QPen(Qt::blue, 2)); // 蓝色边框，宽度 2px
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(opt.rect.adjusted(1, 1, -1, -1)); // 边框内缩 1px，避免覆盖内容
        }

//        // 4. 绘制图片（固定大小 200x150，四周留边 5px）
        QRect imageRect = opt.rect.adjusted(5, 5, -5, -5);
        imageRect.setHeight(150);

        QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(imageRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QRect targetRect = QRect(imageRect.topLeft(), pixmap.size()).adjusted(0, 0, 0, 0);
            painter->drawPixmap(targetRect, pixmap, pixmap.rect());
        }

        // 5. 绘制文字（图片下方，高度 20px）
        QRect textRect = opt.rect;
        textRect.setTop(imageRect.bottom() + 5);
        textRect.setHeight(20);
        textRect.adjust(5, 0, -5, 0);

        QString text = index.data(Qt::DisplayRole).toString();
        painter->setPen(Qt::white);
        painter->drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);

        painter->restore();


        // 调用基类绘制（应用 QSS 样式）
        //QStyledItemDelegate::paint(painter, opt, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        Q_UNUSED(option)
        Q_UNUSED(index)
        return QSize(DITEM_WIDTH + 10, DITEM_HEIGHT + 5 + 20 + 5); // 210x180
    }
};


class LQListView : public QListView
{
    Q_OBJECT
public:
    explicit LQListView(QWidget *parent = nullptr);
     ~LQListView();

    void clearExpiredSessions();
    // 设置为水平流动模式（自动换行 + 自适应宽度）
    void setHorizontalFlowMode();

    // 设置为默认垂直列表模式（单行一项 + 固定布局）
    void setVerticalListMode();
    void reloadOneImage(int row,QString filePathName);
    void slot_loadImagesFromDirectory(QString dirPath);

public slots:
    //void slot_loadImagesFromDirectory(const QModelIndex &index);
    void slot_onImageLoaded(int index,const QString &sessionId, const QString &filePath, const QPixmap &pixmap);
signals:
    void signal_itemSelectChange(QString filePath);
private:
     QStandardItemModel *m_listModel;
    // 会话管理：跟踪当前活动的加载会话
    QMap<QString, LoadSession> m_activeSessions; // 此处定义
    QMutex m_sessionMutex; // 保护 activeSessions 的线程安全

};

#endif // LQLISTVIEW_H
