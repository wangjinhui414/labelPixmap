#ifndef LQTREEVIEW_H
#define LQTREEVIEW_H

#include <QObject>
#include <QTreeView>
#include <QFileSystemModel>

class CustomFileSystemModel : public QFileSystemModel {
public:
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            switch (section) {
                case 0: return "文件名";
                case 1: return "大小";
                case 2: return "类型";
                case 3: return "修改日期";
            }
        }
        return QFileSystemModel::headerData(section, orientation, role);
    }
};

class LQTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit LQTreeView(QWidget *parent = nullptr);
     ~LQTreeView();
    void createWidgets();
    CustomFileSystemModel *fileModel(){return m_fileModel;}
signals:

private:
    CustomFileSystemModel *m_fileModel;
    QStringList openedDirectories; // 保存所有打开的目录
};

#endif // LQTREEVIEW_H
