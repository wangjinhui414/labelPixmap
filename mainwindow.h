#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSplitter>
#include "view/lqtreeview.h"
#include "view/lqlistview.h"
#include "view/simpleinheritance.h"
#include "graphic/lqgraphicsscene.h"
#include "graphic/lqgraphicsview.h"
#include <QListWidget>
#include "global_data.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void createWidges();
    void createMenus();
    void createToolBars();
    void setupToolBarIcons();
    void restoreLastWorkPath();
    QPixmap captureScene(QGraphicsScene* scene);
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);
public slots:
    //文件
    void slot_act_openDirectory();
    void slot_act_closeDirectory();
    //图片批管理
    void slot_act_manage_mkdir();
    void slot_act_manage_delete();
    void slot_act_manage_rename();
    void slot_act_manage_rotate90r();
    void slot_act_rotate90l();
    void slot_act_manage_copy();
    void slot_act_manage_move();
    void slot_act_manage_cut();
    //标注
    void slot_act_editPixMap(QString filePath);
    void slot_act_rect();
    void slot_act_polygon();
    void slot_act_rectRoateR2();
    void slot_act_rectRoateL2();
    void slot_act_rectRoateR01();
    void slot_act_rectRoateL01();
    void slot_act_rectWidthAdd();
    void slot_act_rectWidthSub();
    void slot_act_rectHeightAdd();
    void slot_act_rectHeightSub();
    void slot_act_rectCopy();
    void slot_act_rectPaste();
    void slot_act_selectClass();
    void slot_act_setting();
    void slot_act_viewZoonIn();
    void slot_act_viewZoonOut();
    void slot_act_deleteItem();
    void slot_act_saveLableInfo();
    void slot_act_prevPix();
    void slot_act_nextPix();
    void slot_act_addEll();
    //视图
    void slot_act_modeMange();
    void slot_act_modeLabel();
    //导出
    void slot_act_dataCOCO();
    void slot_act_dataVOC();
    void slot_act_dataYOLO();
    //scene鼠标位置
    void slot_postion(QPointF ps);
    void slot_changeTheme(QString themeStr);
    void slot_changeLang(QString langStr);
protected:
    void fun_rotate_select_image(int arg);
    void pixmapCutScal(int cutmode,qreal x,qreal y,qreal w,qreal h,bool doscal,int scalW,int scalH);
private:
    //菜单
    QMenu *fileMenu;
    QMenu *manageMenu;
    QMenu *annotateMenu;
    QMenu *exportMenu;
    QMenu *viewMenu;
    QMenu *scriptMenu;
    QMenu *helpMenu;
    QToolBar* fileToolBar;
    QToolBar* manageToolBar;
    QToolBar* annotateToolBar;
    QToolBar* viewToolBar;
    QToolBar* exportToolBar;
    QToolBar* scriptToolBar;
    // 工具栏动作组
    QActionGroup *toolBarActions;
    //视图
    LQTreeView *m_treeView;
    LQListView *m_listView;
    LQGraphicsScene *m_sence;
    LQGraphicsView *m_view;
    ImageLabel *m_slLabel;
    //QListWidget *m_listPixInfo;
    QListWidget *m_listWidgeLabels;
    QLabel *m_labelMouse;
    QLabel *m_workPathShow;
    //DOCK
    QDockWidget *m_dockFileTree;
    QDockWidget *m_dockImageList;
    QDockWidget *m_dockslLabel;
    QDockWidget *m_dockPixInfo;
    //其他
    QMap<QString,QGraphicsItem*> m_mapItemInfo;

    QTranslator m_translator;
};
#endif // MAINWINDOW_H
