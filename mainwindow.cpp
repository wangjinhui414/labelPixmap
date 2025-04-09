#include "mainwindow.h"
#include <QDockWidget>
#include <QFileDialog>
#include <QDebug>
#include <QImageReader>
#include <QGraphicsPixmapItem>
#include <QInputDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include "vocexport.h"
#include <QSpinBox>
#include <QFormLayout>
#include <QCheckBox>
#include "yoloexport.h"
#include <QTranslator>
#include <QActionGroup>


QString  getQSSFile(QString fileName)
{
    QFile infile(fileName);
    infile.open(QIODevice::ReadOnly);
    return infile.readAll();
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    golbalState.readSettingIni();
    slot_changeTheme(golbalState.themeStr);
    slot_changeLang(golbalState.langStr);
    createMenus();
    createWidges();
    createToolBars();
    //setupToolBarIcons();
    restoreLastWorkPath();

}

MainWindow::~MainWindow()
{
}

QPixmap MainWindow::captureScene(QGraphicsScene* scene)
{
    if (!scene) {
        qWarning() << "Scene is null!";
        return QPixmap();
    }
    QGraphicsPixmapItem *pixitem = nullptr;
    QList<QGraphicsItem *> sitems = scene->items();
    for(int i=0;i!=sitems.size();i++)
    {
        if(sitems[i]->type() == QGraphicsPixmapItem::Type)
        {
            pixitem = static_cast<QGraphicsPixmapItem*>(sitems[i]);
        }
        if(sitems[i]->type() == LQGraphicsRectItem::Type || sitems[i]->type() == LQGraphicsPathItem::Type)
        {
            auto item1 = static_cast<LQGraphicsRectItem*>(sitems[i]);
            QBrush bsh = item1->brush();
            QColor color1 = bsh.color();
            color1.setAlpha(255);
            bsh.setColor(color1);
            item1->setBrush(bsh);
        }
    }
    QPixmap tmppixmap =  pixitem->pixmap();
    QPixmap pixmapblack(tmppixmap.size());
    pixmapblack.fill(Qt::black);
    pixitem->setPixmap(pixmapblack);

    // 获取场景的矩形区域
    QRectF sceneRect = scene->sceneRect();

    // 创建一个与场景大小相同的 QPixmap
    QPixmap pixmap(sceneRect.size().toSize());
    pixmap.fill(Qt::transparent); // 设置背景为透明

    // 创建一个 QPainter，用于将场景渲染到 QPixmap
    QPainter painter(&pixmap);
    scene->render(&painter, QRectF(), sceneRect);
    pixitem->setPixmap(tmppixmap);
    for(int i=0;i!=sitems.size();i++)
    {
        if(sitems[i]->type() == LQGraphicsRectItem::Type || sitems[i]->type() == LQGraphicsPathItem::Type)
        {
            auto item1 = static_cast<LQGraphicsRectItem*>(sitems[i]);
            QBrush bsh = item1->brush();
            QColor color1 = bsh.color();
            color1.setAlpha(golbalState.alpha);
            bsh.setColor(color1);
            item1->setBrush(bsh);
        }
    }
    return pixmap;
}

void MainWindow::createWidges()
{
    m_treeView = new LQTreeView(this);
    m_listView = new LQListView(this);
    m_sence = new LQGraphicsScene(this);
    m_view = new LQGraphicsView(this);
    m_view->setScene(m_sence);
    m_slLabel = new ImageLabel;
    m_slLabel->setMinimumHeight(70);
    m_listWidgeLabels = new QListWidget;

    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    setCentralWidget(m_view);
    // 创建左侧停靠窗口
    m_dockFileTree = new QDockWidget(tr("文件浏览器"), this);
    m_dockFileTree->setWidget(m_treeView); // 将树视图添加到停靠窗口
    m_dockFileTree->setAllowedAreas(Qt::LeftDockWidgetArea); // 允许停靠在左侧或右侧
    addDockWidget(Qt::LeftDockWidgetArea, m_dockFileTree); // 将停靠窗口添加到主窗口左侧

    m_dockImageList = new QDockWidget(tr("图片浏览器"), this);
    m_dockImageList->setWidget(m_listView); // 将树视图添加到停靠窗口
    m_dockImageList->setAllowedAreas(Qt::LeftDockWidgetArea); // 允许停靠在左侧或右侧
    addDockWidget(Qt::LeftDockWidgetArea, m_dockImageList); // 将停靠窗口添加到主窗口左侧
    // 并列显示两个 QDockWidget
    splitDockWidget(m_dockFileTree, m_dockImageList, Qt::Horizontal);
    // 禁用共享分割条
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);

    m_dockslLabel = new QDockWidget(tr("标注缩略图"), this);
    m_dockslLabel->setWidget(m_slLabel); // 将树视图添加到停靠窗口
    m_dockslLabel->setAllowedAreas(Qt::RightDockWidgetArea); // 允许停靠在左侧或右侧
    addDockWidget(Qt::RightDockWidgetArea, m_dockslLabel); // 将停靠窗口添加到主窗口左侧

    m_dockPixInfo = new QDockWidget(tr("标注信息"), this);
    m_dockPixInfo->setWidget(m_listWidgeLabels); // 将树视图添加到停靠窗口
    m_dockPixInfo->setAllowedAreas(Qt::RightDockWidgetArea); // 允许停靠在左侧或右侧
    addDockWidget(Qt::RightDockWidgetArea, m_dockPixInfo); // 将停靠窗口添加到主窗口左侧

    viewMenu->addAction(m_dockFileTree->toggleViewAction());
    viewMenu->addAction(m_dockImageList->toggleViewAction());
    viewMenu->addAction(m_dockslLabel->toggleViewAction());
    viewMenu->addAction(m_dockPixInfo->toggleViewAction());


    connect(m_treeView,&LQTreeView::doubleClicked,this,[this](const QModelIndex index){
        auto fileModel = static_cast<LQTreeView*>(sender())->fileModel();
        if (!fileModel || !fileModel->isDir(index)) return;
        golbalState.editPath = fileModel->filePath(index);
        m_listView->slot_loadImagesFromDirectory(golbalState.editPath);
        //qDebug()<<golbalState.editPath;
        m_workPathShow->setText(golbalState.editPath);
    });
    connect(m_listView,&LQListView::signal_itemSelectChange,this,&MainWindow::slot_act_editPixMap);
    connect(m_listWidgeLabels,&QListWidget::currentItemChanged,this,[this](QListWidgetItem *item){
        if(item==nullptr) return;
        if(!m_mapItemInfo.contains(item->text())) return;
        auto itemRect = m_mapItemInfo[item->text()];
        QList<QGraphicsItem *> sitems = m_sence->selectedItems();
        for(int i=0;i!=sitems.size();i++)
        {
            if(sitems[i]->type() == LQGraphicsRectItem::Type || sitems[i]->type() == LQGraphicsPathItem::Type)
            {
                QGraphicsItem *rectItem = static_cast<QGraphicsItem*>(sitems[i]);
                rectItem->setSelected(false);
            }
        }
        itemRect->setSelected(true);
    });
    m_labelMouse = new QLabel();
    QString strShow = QString("X: %1pix Y: %2pix").arg(0, 4, 10, QChar('0'))
                                    .arg(0, 4, 10, QChar('0'));

    m_labelMouse->setText(strShow);
    m_workPathShow = new QLabel();
    m_workPathShow->setTextInteractionFlags(Qt::TextSelectableByMouse);

    statusBar()->addPermanentWidget(m_workPathShow);

    statusBar()->addPermanentWidget(m_labelMouse);
    connect(m_sence, &LQGraphicsScene::signal_sendPosition, this, &MainWindow::slot_postion);

    connect(m_listView, &LQListView::customContextMenuRequested,this,[this](const QPoint &pos){
        QPoint globalPos = m_listView->viewport()->mapToGlobal(pos);
        manageMenu->exec(globalPos);
    });
    QIcon appIcon(":/images/iPhotoDraw.ico");  // 图标路径，资源文件路径
    setWindowIcon(appIcon);
    setWindowTitle(tr("图片标注软件V1.05"));
    showMaximized();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    resizeDocks({m_dockFileTree, m_dockImageList}, {this->width()/8, this->width()*29/80}, Qt::Horizontal);
    resizeDocks({m_dockslLabel, m_dockPixInfo}, {this->width()/8, this->width()/8}, Qt::Horizontal);
    resizeDocks({m_dockslLabel, m_dockPixInfo}, {this->height()/5, this->height()*4/5}, Qt::Vertical);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    golbalState.writeSettingIni();
    QMainWindow::closeEvent(event);
}


void MainWindow::createMenus()
{
    // 创建菜单栏
    QMenuBar *menuBar = this->menuBar();
    // 文件菜单
    fileMenu = menuBar->addMenu(tr("文件"));
    QAction *act_openDirAction = new QAction(tr("打开目录"), this);
    act_openDirAction->setObjectName("act_openDirAction");
    fileMenu->addAction(act_openDirAction);

    QAction *act_closeDirAction = new QAction(tr("关闭目录"), this);
    act_closeDirAction->setObjectName("act_closeDirAction");
    fileMenu->addAction(act_closeDirAction);

    fileMenu->addSeparator();

    QAction *act_exitAction = new QAction(tr("退出"), this);
    act_exitAction->setObjectName("act_exitAction");
    fileMenu->addAction(act_exitAction);

    // 管理菜单
    manageMenu = menuBar->addMenu(tr("管理"));
    QAction *act_mkdir = new QAction(tr("创建目录"), this);
    act_mkdir->setObjectName("act_mkdir");
    act_mkdir->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_M));
    manageMenu->addAction(act_mkdir);

    QAction *act_mgdel = new QAction(tr("删除图片"), this);
    act_mgdel->setObjectName("act_mgdel");
    act_mgdel->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_D));
    manageMenu->addAction(act_mgdel);

    QAction *act_mgrename = new QAction(tr("重命名"), this);
    act_mgrename->setObjectName("act_mgrename");
    act_mgrename->setShortcut(QKeySequence(Qt::ShiftModifier |Qt::Key_N));
    manageMenu->addAction(act_mgrename);

    QAction *act_mgrotate90r = new QAction(tr("顺时针旋转90°"), this);
    act_mgrotate90r->setObjectName("act_mgrotate90r");
    act_mgrotate90r->setShortcut(QKeySequence(Qt::ShiftModifier |Qt::Key_R));
    manageMenu->addAction(act_mgrotate90r);

    QAction *act_mgrotate90l = new QAction(tr("逆时针旋转90°"), this);
    act_mgrotate90l->setObjectName("act_mgrotate90l");
    act_mgrotate90l->setShortcut(QKeySequence(Qt::ShiftModifier |Qt::Key_L));
    manageMenu->addAction(act_mgrotate90l);

    QAction *act_mgcopy = new QAction(tr("复制图片到"), this);
    act_mgcopy->setObjectName("act_mgcopy");
    act_mgcopy->setShortcut(QKeySequence(Qt::ShiftModifier |Qt::Key_C));
    manageMenu->addAction(act_mgcopy);

    QAction *act_mgmove = new QAction(tr("移动图片到"), this);
    act_mgmove->setObjectName("act_mgmove");
    act_mgmove->setShortcut(QKeySequence(Qt::ShiftModifier |Qt::Key_V));
    manageMenu->addAction(act_mgmove);

    QAction *act_mgcut = new QAction(tr("剪裁图片"), this);
    act_mgcut->setObjectName("act_mgcut");
    act_mgcut->setShortcut(QKeySequence(Qt::ShiftModifier |Qt::Key_U));
    manageMenu->addAction(act_mgcut);

    // 标注菜单
    annotateMenu = menuBar->addMenu(tr("标注"));

    // 添加互斥单选菜单项（一组中只能选一个）
    QActionGroup* radioGroup = new QActionGroup(annotateMenu);
    radioGroup->setExclusive(true);

    QAction* act_rect = new QAction(tr("矩形标注"), radioGroup);
    act_rect->setObjectName("act_rect");
    act_rect->setShortcut(QKeySequence(Qt::Key_F5));
    act_rect->setCheckable(true);

    QAction* act_polygon = new QAction(tr("多边形标注"), radioGroup);
    act_polygon->setObjectName("act_polygon");
    act_polygon->setShortcut(QKeySequence(Qt::Key_F6));
    act_polygon->setCheckable(true);

    if(golbalState.labelMode==0)
        act_rect->setChecked(true);
    else
        act_polygon->setChecked(true);
    annotateMenu->addActions(radioGroup->actions());


    annotateMenu->addSeparator();

    QAction *act_rectRoateR2 = new QAction(tr("矩形右旋"), this);
    act_rectRoateR2->setObjectName("act_rectRoateR2");
    act_rectRoateR2->setShortcut(QKeySequence(Qt::Key_R));
    annotateMenu->addAction(act_rectRoateR2);

    QAction *act_rectRoateL2 = new QAction(tr("矩形左旋"), this);
    act_rectRoateL2->setObjectName("act_rectRoateL2");
    act_rectRoateL2->setShortcut(QKeySequence(Qt::Key_Tab));
    annotateMenu->addAction(act_rectRoateL2);

    QAction *act_rectRoateR01 = new QAction(tr("矩形右旋微调"), this);
    act_rectRoateR01->setObjectName("act_rectRoateR01");
    act_rectRoateR01->setShortcut(QKeySequence(Qt::Key_E));
    annotateMenu->addAction(act_rectRoateR01);

    QAction *act_rectRoateL01 = new QAction(tr("矩形左旋微调"), this);
    act_rectRoateL01->setObjectName("act_rectRoateL01");
    act_rectRoateL01->setShortcut(QKeySequence(Qt::Key_Q));
    annotateMenu->addAction(act_rectRoateL01);

    QAction *act_rectWidthAdd = new QAction(tr("矩形宽度增加"), this);
    act_rectWidthAdd->setObjectName("act_rectWidthAdd");
    act_rectWidthAdd->setShortcut(QKeySequence(Qt::Key_D));
    annotateMenu->addAction(act_rectWidthAdd);

    QAction *act_rectWidthSub = new QAction(tr("矩形宽度减小"), this);
    act_rectWidthSub->setObjectName("act_rectWidthSub");
    act_rectWidthSub->setShortcut(QKeySequence(Qt::Key_A));
    annotateMenu->addAction(act_rectWidthSub);

    QAction *act_rectHeightAdd = new QAction(tr("矩形高度增加"), this);
    act_rectHeightAdd->setObjectName("act_rectHeightAdd");
    act_rectHeightAdd->setShortcut(QKeySequence(Qt::Key_W));
    annotateMenu->addAction(act_rectHeightAdd);

    QAction *act_rectHeightSub = new QAction(tr("矩形高度减小"), this);
    act_rectHeightSub->setObjectName("act_rectHeightSub");
    act_rectHeightSub->setShortcut(QKeySequence(Qt::Key_S));
    annotateMenu->addAction(act_rectHeightSub);

    QAction *act_rectCopy = new QAction(tr("复制矩形"), this);
    act_rectCopy->setObjectName("act_rectCopy");
    act_rectCopy->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    annotateMenu->addAction(act_rectCopy);

    QAction *act_rectPaste = new QAction(tr("粘贴矩形"), this);
    act_rectPaste->setObjectName("act_rectPaste");
    act_rectPaste->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
    annotateMenu->addAction(act_rectPaste);

    annotateMenu->addSeparator();

    QAction *act_selectClass = new QAction(tr("修改图元类别"), this);
    act_selectClass->setObjectName("act_selectClass");
    act_selectClass->setShortcut(QKeySequence(Qt::Key_T));
    annotateMenu->addAction(act_selectClass);

    QAction *act_addEll = new QAction(tr("多边形点添加"), this);
    act_addEll->setObjectName("act_addEll");
    act_addEll->setShortcut(QKeySequence(Qt::Key_B));
    annotateMenu->addAction(act_addEll);

    QAction *act_deleteItem = new QAction(tr("图元删除"), this);
    act_deleteItem->setObjectName("act_deleteItem");
    act_deleteItem->setShortcut(QKeySequence(Qt::Key_X));
    annotateMenu->addAction(act_deleteItem);

    QAction *act_setting = new QAction(tr("参数设置"), this);
    act_setting->setObjectName("act_setting");
    act_setting->setShortcut(QKeySequence(Qt::Key_P));
    annotateMenu->addAction(act_setting);

    QAction *act_viewZoonIn = new QAction(tr("视图缩小"), this);
    act_viewZoonIn->setObjectName("act_viewZoonIn");
    act_viewZoonIn->setShortcut(QKeySequence(Qt::Key_Minus));
    annotateMenu->addAction(act_viewZoonIn);

    QAction *act_viewZoonOut = new QAction(tr("视图放大"), this);
    act_viewZoonOut->setObjectName("act_viewZoonOut");
    act_viewZoonOut->setShortcut(QKeySequence(Qt::Key_Plus));
    annotateMenu->addAction(act_viewZoonOut);

    QAction *act_saveLableInfo = new QAction(tr("保存标注信息"), this);
    act_saveLableInfo->setObjectName("act_saveLableInfo");
    act_saveLableInfo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    annotateMenu->addAction(act_saveLableInfo);

    QAction *act_prevPix = new QAction(tr("上一张"), this);
    act_prevPix->setObjectName("act_prevPix");
    act_prevPix->setShortcut(QKeySequence(Qt::Key_F1));
    annotateMenu->addAction(act_prevPix);

    QAction *act_nextPix = new QAction(tr("下一张"), this);
    act_nextPix->setObjectName("act_nextPix");
    act_nextPix->setShortcut(QKeySequence(Qt::Key_F2));
    annotateMenu->addAction(act_nextPix);

    annotateMenu->addSeparator();

    // 导出菜单
    exportMenu = menuBar->addMenu(tr("导出"));
    QAction *act_dataCOCO = new QAction(tr("导出COCO格式数据"), this);
    act_dataCOCO->setObjectName("act_dataCOCO");
    exportMenu->addAction(act_dataCOCO);
    act_dataCOCO->setVisible(false);

    QAction *act_dataVOC = new QAction(tr("导出VOC格式数据"), this);
    act_dataVOC->setObjectName("act_dataVOC");
    exportMenu->addAction(act_dataVOC);

    QAction *act_dataYOLO = new QAction(tr("导出YOLO格式数据"), this);
    act_dataYOLO->setObjectName("act_dataYOLO");
    exportMenu->addAction(act_dataYOLO);

    // 视图菜单
    viewMenu = menuBar->addMenu(tr("视图"));

    // 添加互斥单选菜单项（一组中只能选一个）
    QActionGroup* radioGroup1 = new QActionGroup(viewMenu);
    radioGroup1->setExclusive(true);

    QAction* act_modeMange = new QAction(tr("管理模式"), radioGroup1);
    act_modeMange->setObjectName("act_modeMange");
    act_modeMange->setShortcut(QKeySequence(Qt::Key_F3));
    act_modeMange->setCheckable(true);

    QAction* act_modeLabel = new QAction(tr("标注模式"), radioGroup1);
    act_modeLabel->setObjectName("act_modeLabel");
    act_modeLabel->setShortcut(QKeySequence(Qt::Key_F4));
    act_modeLabel->setCheckable(true);

    act_modeMange->setChecked(true);
    viewMenu->addActions(radioGroup1->actions());
    viewMenu->addSeparator();

    // 脚本菜单
//    scriptMenu = menuBar->addMenu(tr("python脚本"));
//    QAction *act_testScript = new QAction(tr("测试脚本"), this);
//    act_testScript->setObjectName("act_testScript");
//    scriptMenu->addAction(act_testScript);

    // 工具菜单 - 用于管理工具栏显示
    QMenu *toolsMenu = menuBar->addMenu(tr("工具"));

    // 帮助菜单
    helpMenu = menuBar->addMenu(tr("其他"));

    QActionGroup* radioGroupTheme = new QActionGroup(helpMenu);
    radioGroupTheme->setExclusive(true);
    QAction *act_themeDark = new QAction(tr("黑色主题"), radioGroupTheme);
    act_themeDark->setObjectName("act_themeDark");
    act_themeDark->setCheckable(true);
    QAction *act_themeWhite = new QAction(tr("白色主题"), radioGroupTheme);
    act_themeWhite->setObjectName("act_themeWhite");
    act_themeWhite->setCheckable(true);
    connect(act_themeDark, &QAction::triggered, this,  [this](){
        golbalState.themeStr = "dark";
        slot_changeTheme(golbalState.themeStr);
    });

    connect(act_themeWhite, &QAction::triggered, this,  [this](){
        golbalState.themeStr = "white";
        slot_changeTheme(golbalState.themeStr);
    });
    if(golbalState.themeStr=="dark")
        act_themeDark->setChecked(true);
    else
        act_themeWhite->setChecked(true);
    helpMenu->addActions(radioGroupTheme->actions());
    helpMenu->addSeparator();



    QActionGroup* radioGroupLang = new QActionGroup(helpMenu);
    radioGroupLang->setExclusive(true);
    QAction *act_langCN = new QAction("中文(需要重启)", radioGroupLang);
    act_langCN->setObjectName("act_langCN");
    act_langCN->setCheckable(true);
    QAction *act_langEN = new QAction("English(Restart to apply)", radioGroupLang);
    act_langEN->setObjectName("act_langEN");
    act_langEN->setCheckable(true);
    connect(act_langCN, &QAction::triggered, this,  [this](){
        golbalState.langStr = "zh";
        slot_changeLang(golbalState.langStr);
    });
    connect(act_langEN, &QAction::triggered, this,  [this](){
        golbalState.langStr = "en";
        slot_changeLang(golbalState.langStr);
    });
    if(golbalState.langStr=="zh")
        act_langCN->setChecked(true);
    else
        act_langEN->setChecked(true);

    helpMenu->addActions(radioGroupLang->actions());
    helpMenu->addSeparator();
    QAction *act_keyHelp = new QAction(tr("快捷键帮助"), this);
    act_keyHelp->setObjectName("act_keyHelp");
    helpMenu->addAction(act_keyHelp);

    // 连接动作信号到槽函数
    // 文件
    connect(act_openDirAction, &QAction::triggered, this, &MainWindow::slot_act_openDirectory);
    connect(act_closeDirAction, &QAction::triggered, this, &MainWindow::slot_act_closeDirectory);
    connect(act_exitAction, &QAction::triggered, this, &MainWindow::close);

    // 管理
    connect(act_mkdir, &QAction::triggered, this, &MainWindow::slot_act_manage_mkdir);
    connect(act_mgdel, &QAction::triggered, this, &MainWindow::slot_act_manage_delete);
    connect(act_mgrename, &QAction::triggered, this, &MainWindow::slot_act_manage_rename);
    connect(act_mgrotate90r, &QAction::triggered, this, &MainWindow::slot_act_manage_rotate90r);
    connect(act_mgrotate90l, &QAction::triggered, this, &MainWindow::slot_act_rotate90l);
    connect(act_mgcopy, &QAction::triggered, this, &MainWindow::slot_act_manage_copy);
    connect(act_mgmove, &QAction::triggered, this, &MainWindow::slot_act_manage_move);
    connect(act_mgcut, &QAction::triggered, this, &MainWindow::slot_act_manage_cut);

    // 标注
    connect(act_rect,          &QAction::triggered, this, &MainWindow::slot_act_rect);
    connect(act_polygon,       &QAction::triggered, this, &MainWindow::slot_act_polygon);
    connect(act_setting,       &QAction::triggered, this, &MainWindow::slot_act_setting);
    connect(act_rectRoateR2,   &QAction::triggered, this, &MainWindow::slot_act_rectRoateR2);
    connect(act_rectRoateL2,   &QAction::triggered, this, &MainWindow::slot_act_rectRoateL2);
    connect(act_rectRoateR01,  &QAction::triggered, this, &MainWindow::slot_act_rectRoateR01);
    connect(act_rectRoateL01,  &QAction::triggered, this, &MainWindow::slot_act_rectRoateL01);
    connect(act_rectWidthAdd,  &QAction::triggered, this, &MainWindow::slot_act_rectWidthAdd);
    connect(act_rectWidthSub,  &QAction::triggered, this, &MainWindow::slot_act_rectWidthSub);
    connect(act_rectHeightAdd, &QAction::triggered, this, &MainWindow::slot_act_rectHeightAdd);
    connect(act_rectHeightSub, &QAction::triggered, this, &MainWindow::slot_act_rectHeightSub);
    connect(act_rectCopy,      &QAction::triggered, this, &MainWindow::slot_act_rectCopy);
    connect(act_rectPaste,     &QAction::triggered, this, &MainWindow::slot_act_rectPaste);
    connect(act_selectClass,   &QAction::triggered, this, &MainWindow::slot_act_selectClass);
    connect(act_viewZoonIn,    &QAction::triggered, this, &MainWindow::slot_act_viewZoonIn);
    connect(act_viewZoonOut,   &QAction::triggered, this, &MainWindow::slot_act_viewZoonOut);
    connect(act_deleteItem,    &QAction::triggered, this, &MainWindow::slot_act_deleteItem);
    connect(act_saveLableInfo, &QAction::triggered, this, &MainWindow::slot_act_saveLableInfo);
    connect(act_prevPix,       &QAction::triggered, this, &MainWindow::slot_act_prevPix);
    connect(act_nextPix,       &QAction::triggered, this, &MainWindow::slot_act_nextPix);
    connect(act_addEll,        &QAction::triggered, this, &MainWindow::slot_act_addEll);

    // 视图
    connect(act_modeMange, &QAction::triggered, this, &MainWindow::slot_act_modeMange);
    connect(act_modeLabel, &QAction::triggered, this, &MainWindow::slot_act_modeLabel);
    //数据导出
    connect(act_dataCOCO, &QAction::triggered, this, &MainWindow::slot_act_dataCOCO);
    connect(act_dataVOC, &QAction::triggered, this, &MainWindow::slot_act_dataVOC);
    connect(act_dataYOLO, &QAction::triggered, this, &MainWindow::slot_act_dataYOLO);


    // 帮助
    connect(act_keyHelp, &QAction::triggered, this,[](){
        QLabel *label = new QLabel(tr("1、矩形绘图中，ALT和鼠标移动块快速调整大小。\n"
                                   "2、多边形绘图中,快捷键B可以添加折线点。"));
        label->show();
    });


    // 创建工具栏显示/隐藏的动作
    toolBarActions = new QActionGroup(this);
    toolBarActions->setExclusive(false);

    QAction *showFileToolBar = new QAction(tr("文件工具栏"), this);
    showFileToolBar->setObjectName("act_showFileToolBar");
    showFileToolBar->setCheckable(true);
    showFileToolBar->setChecked(true);
    toolBarActions->addAction(showFileToolBar);

    QAction *showManageToolBar = new QAction(tr("管理工具栏"), this);
    showManageToolBar->setObjectName("act_showManageToolBar");
    showManageToolBar->setCheckable(true);
    showManageToolBar->setChecked(true);
    toolBarActions->addAction(showManageToolBar);

    QAction *showAnnotateToolBar = new QAction(tr("标注工具栏"), this);
    showAnnotateToolBar->setObjectName("act_showAnnotateToolBar");
    showAnnotateToolBar->setCheckable(true);
    showAnnotateToolBar->setChecked(true);
    toolBarActions->addAction(showAnnotateToolBar);

    QAction *showViewToolBar = new QAction(tr("视图工具栏"), this);
    showViewToolBar->setObjectName("act_showViewToolBar");
    showViewToolBar->setCheckable(true);
    showViewToolBar->setChecked(true);
    toolBarActions->addAction(showViewToolBar);

    QAction *showExportToolBar = new QAction(tr("导出工具栏"), this);
    showExportToolBar->setObjectName("act_showExportToolBar");
    showExportToolBar->setCheckable(true);
    showExportToolBar->setChecked(true);
    toolBarActions->addAction(showExportToolBar);

//    QAction *showScriptToolBar = new QAction(tr("脚本工具栏"), this);
//    showScriptToolBar->setObjectName("act_showScriptToolBar");
//    showScriptToolBar->setCheckable(true);
//    showScriptToolBar->setChecked(true);
//    toolBarActions->addAction(showScriptToolBar);

    // 添加到工具菜单
    toolsMenu->addActions(toolBarActions->actions());

    // 连接信号槽
    connect(showFileToolBar, &QAction::toggled, this, [this](bool visible){
        fileToolBar->setVisible(visible);
    });
    connect(showManageToolBar, &QAction::toggled, this,  [this](bool visible){
        manageToolBar->setVisible(visible);
    });
    connect(showAnnotateToolBar, &QAction::toggled, this, [this](bool visible){
        annotateToolBar->setVisible(visible);
    });
    connect(showViewToolBar, &QAction::toggled, this,  [this](bool visible){
        viewToolBar->setVisible(visible);
    });
    connect(showExportToolBar, &QAction::toggled, this,  [this](bool visible){
        exportToolBar->setVisible(visible);
    });
//    connect(showScriptToolBar, &QAction::toggled, this,  [this](bool visible){
//        scriptToolBar->setVisible(visible);
//    });




}

void MainWindow::createToolBars()
{
    fileToolBar    = new QToolBar;
    manageToolBar  = new QToolBar;
    annotateToolBar= new QToolBar;
    viewToolBar    = new QToolBar;
    exportToolBar  = new QToolBar;
    scriptToolBar  = new QToolBar;


    // 文件操作工具栏
    fileToolBar = addToolBar(tr("文件"));
    fileToolBar->setObjectName("FileToolBar");
    fileToolBar->addAction(findChild<QAction*>("act_openDirAction"));
    fileToolBar->addAction(findChild<QAction*>("act_closeDirAction"));
    fileToolBar->addSeparator();
    fileToolBar->addAction(findChild<QAction*>("act_exitAction"));

    // 图片管理工具栏
    manageToolBar = addToolBar(tr("管理"));
    manageToolBar->setObjectName("ManageToolBar");
    manageToolBar->addAction(findChild<QAction*>("act_mkdir"));
    manageToolBar->addAction(findChild<QAction*>("act_mgdel"));
    manageToolBar->addAction(findChild<QAction*>("act_mgrename"));
    manageToolBar->addSeparator();
    manageToolBar->addAction(findChild<QAction*>("act_mgrotate90r"));
    manageToolBar->addAction(findChild<QAction*>("act_mgrotate90l"));
    manageToolBar->addSeparator();
    manageToolBar->addAction(findChild<QAction*>("act_mgcopy"));
    manageToolBar->addAction(findChild<QAction*>("act_mgmove"));
    manageToolBar->addSeparator();
    manageToolBar->addAction(findChild<QAction*>("act_mgcut"));

    // 标注工具栏
    annotateToolBar = addToolBar(tr("标注"));
    annotateToolBar->setObjectName("AnnotateToolBar");

    // 标注模式按钮组
    annotateToolBar->addAction(findChild<QAction*>("act_rect"));
    annotateToolBar->addAction(findChild<QAction*>("act_polygon"));

    annotateToolBar->addSeparator();
    annotateToolBar->addAction(findChild<QAction*>("act_rectRoateR2"));
    annotateToolBar->addAction(findChild<QAction*>("act_rectRoateL2"));
    annotateToolBar->addAction(findChild<QAction*>("act_rectRoateR01"));
    annotateToolBar->addAction(findChild<QAction*>("act_rectRoateL01"));
    annotateToolBar->addSeparator();
    annotateToolBar->addAction(findChild<QAction*>("act_rectWidthAdd"));
    annotateToolBar->addAction(findChild<QAction*>("act_rectWidthSub"));
    annotateToolBar->addAction(findChild<QAction*>("act_rectHeightAdd"));
    annotateToolBar->addAction(findChild<QAction*>("act_rectHeightSub"));
    annotateToolBar->addSeparator();
    annotateToolBar->addAction(findChild<QAction*>("act_rectCopy"));
    annotateToolBar->addAction(findChild<QAction*>("act_rectPaste"));
    annotateToolBar->addSeparator();
    annotateToolBar->addAction(findChild<QAction*>("act_selectClass"));
    annotateToolBar->addAction(findChild<QAction*>("act_addEll"));
    annotateToolBar->addAction(findChild<QAction*>("act_deleteItem"));
    annotateToolBar->addSeparator();
    annotateToolBar->addAction(findChild<QAction*>("act_setting"));
    annotateToolBar->addAction(findChild<QAction*>("act_saveLableInfo"));
    annotateToolBar->addSeparator();
    annotateToolBar->addAction(findChild<QAction*>("act_viewZoonIn"));
    annotateToolBar->addAction(findChild<QAction*>("act_viewZoonOut"));
    annotateToolBar->addSeparator();
    annotateToolBar->addAction(findChild<QAction*>("act_prevPix"));
    annotateToolBar->addAction(findChild<QAction*>("act_nextPix"));
    // 视图操作工具栏
    viewToolBar = addToolBar(tr("视图"));
    viewToolBar->setObjectName("ViewToolBar");

    viewToolBar->addAction(findChild<QAction*>("act_modeMange"));
    viewToolBar->addAction(findChild<QAction*>("act_modeLabel"));


    // 导出工具栏
    exportToolBar = addToolBar(tr("导出"));
    exportToolBar->setObjectName("ExportToolBar");
    exportToolBar->addAction(findChild<QAction*>("act_dataCOCO"));
    exportToolBar->addAction(findChild<QAction*>("act_dataVOC"));
    exportToolBar->addAction(findChild<QAction*>("act_dataYOLO"));

//    // 脚本工具栏
//    scriptToolBar = addToolBar(tr("python脚本"));
//    scriptToolBar->setObjectName("ScriptToolBar");
//    scriptToolBar->addAction(findChild<QAction*>("act_testScript"));

    // 设置工具栏图标和工具提示
    setupToolBarIcons();
}

void MainWindow::setupToolBarIcons()
{
    // 文件工具栏图标
    findChild<QAction*>("act_openDirAction")->setIcon(QIcon(":/icons/open_folder.png"));
    findChild<QAction*>("act_closeDirAction")->setIcon(QIcon(":/icons/close_folder.png"));
    findChild<QAction*>("act_exitAction")->setIcon(QIcon(":/icons/exit.png"));

    // 管理工具栏图标
    findChild<QAction*>("act_mkdir")->setIcon(QIcon(":/icons/mkdir.png"));
    findChild<QAction*>("act_mgdel")->setIcon(QIcon(":/icons/delete.png"));
    findChild<QAction*>("act_mgrename")->setIcon(QIcon(":/icons/rename.png"));
    findChild<QAction*>("act_mgrotate90r")->setIcon(QIcon(":/icons/image_rotate_right.png"));
    findChild<QAction*>("act_mgrotate90l")->setIcon(QIcon(":/icons/image_rotate_left.png"));
    findChild<QAction*>("act_mgcopy")->setIcon(QIcon(":/icons/copy.png"));
    findChild<QAction*>("act_mgmove")->setIcon(QIcon(":/icons/move.png"));
    findChild<QAction*>("act_mgcut")->setIcon(QIcon(":/icons/cut.png"));

    // 标注工具栏图标
    findChild<QAction*>("act_rect")->setIcon(QIcon(":/icons/rectangle.png"));
    findChild<QAction*>("act_polygon")->setIcon(QIcon(":/icons/polygon.png"));
    findChild<QAction*>("act_rectRoateR2")->setIcon(QIcon(":/icons/rotate_right.png"));
    findChild<QAction*>("act_rectRoateL2")->setIcon(QIcon(":/icons/rotate_left.png"));
    findChild<QAction*>("act_rectRoateR01")->setIcon(QIcon(":/icons/rotate_right_small.png"));
    findChild<QAction*>("act_rectRoateL01")->setIcon(QIcon(":/icons/rotate_left_small.png"));
    findChild<QAction*>("act_rectWidthAdd")->setIcon(QIcon(":/icons/width_increase.png"));
    findChild<QAction*>("act_rectWidthSub")->setIcon(QIcon(":/icons/width_decrease.png"));
    findChild<QAction*>("act_rectHeightAdd")->setIcon(QIcon(":/icons/height_increase.png"));
    findChild<QAction*>("act_rectHeightSub")->setIcon(QIcon(":/icons/height_decrease.png"));
    findChild<QAction*>("act_rectCopy")->setIcon(QIcon(":/icons/copy_rect.png"));
    findChild<QAction*>("act_rectPaste")->setIcon(QIcon(":/icons/paste_rect.png"));
    findChild<QAction*>("act_selectClass")->setIcon(QIcon(":/icons/select_class.png"));
    findChild<QAction*>("act_addEll")->setIcon(QIcon(":/icons/add_point.png"));
    findChild<QAction*>("act_deleteItem")->setIcon(QIcon(":/icons/delete_item.png"));
    findChild<QAction*>("act_setting")->setIcon(QIcon(":/icons/settings.png"));
    findChild<QAction*>("act_saveLableInfo")->setIcon(QIcon(":/icons/save.png"));

    // 视图工具栏图标
    findChild<QAction*>("act_modeMange")->setIcon(QIcon(":/icons/manage_mode.png"));
    findChild<QAction*>("act_modeLabel")->setIcon(QIcon(":/icons/label_mode.png"));
    findChild<QAction*>("act_viewZoonIn")->setIcon(QIcon(":/icons/zoom_out.png"));
    findChild<QAction*>("act_viewZoonOut")->setIcon(QIcon(":/icons/zoom_in.png"));
    findChild<QAction*>("act_prevPix")->setIcon(QIcon(":/icons/previous.png"));
    findChild<QAction*>("act_nextPix")->setIcon(QIcon(":/icons/next.png"));

    // 导出工具栏图标
    findChild<QAction*>("act_dataCOCO")->setIcon(QIcon(":/icons/export_coco.png"));
    findChild<QAction*>("act_dataVOC")->setIcon(QIcon(":/icons/export_voc.png"));
    findChild<QAction*>("act_dataYOLO")->setIcon(QIcon(":/icons/export_yolo.png"));

//    // 脚本工具栏图标
//    findChild<QAction*>("act_testScript")->setIcon(QIcon(":/icons/script.png"));

    // 设置所有工具栏按钮的工具提示（显示功能名称和快捷键）
    QList<QAction*> allActions = findChildren<QAction*>();
    foreach (QAction* action, allActions) {
        if (!action->shortcut().isEmpty()) {
            action->setToolTip(action->text() + " (" + action->shortcut().toString() + ")");
        } else {
            action->setToolTip(action->text());
        }
    }


#define ICO_SIZE 15
    fileToolBar->setIconSize(QSize(ICO_SIZE, ICO_SIZE));
    manageToolBar->setIconSize(QSize(ICO_SIZE, ICO_SIZE));
    annotateToolBar->setIconSize(QSize(ICO_SIZE, ICO_SIZE));
    viewToolBar->setIconSize(QSize(ICO_SIZE, ICO_SIZE));
    exportToolBar->setIconSize(QSize(ICO_SIZE, ICO_SIZE));
    scriptToolBar->setIconSize(QSize(ICO_SIZE, ICO_SIZE));

    // fileToolBar->setMaximumHeight(15);  // 最大宽度和高度
    // manageToolBar->setMaximumHeight(15);  // 最大宽度和高度
    // annotateToolBar->setMaximumHeight(15);  // 最大宽度和高度
    // viewToolBar->setMaximumHeight(15);  // 最大宽度和高度
    // exportToolBar->setMaximumHeight(15);  // 最大宽度和高度
    // scriptToolBar->setMaximumHeight(15);  // 最大宽度和高度
}


void MainWindow::restoreLastWorkPath()
{
    CustomFileSystemModel *fmodel = static_cast<CustomFileSystemModel*>(m_treeView->model());
    m_treeView->setRootIndex(fmodel->index(golbalState.workPath));
    m_treeView->expand(fmodel->index(golbalState.editPath));
    m_listView->slot_loadImagesFromDirectory(golbalState.editPath);
    m_workPathShow->setText(golbalState.editPath);

    QApplication::processEvents(QEventLoop::AllEvents,100);
    QStandardItemModel *modelX = static_cast<QStandardItemModel*>(m_listView->model()) ;
    QApplication::processEvents(QEventLoop::AllEvents,100);
    if(modelX->rowCount()>0){
        m_listView->setCurrentIndex(modelX->index(0,0));
    }
}


void MainWindow::slot_act_openDirectory()
{
    // 1. 让用户选择目标目录
    QString targetDir = QFileDialog::getExistingDirectory(
       nullptr,
       tr("选择目标目录"),
       QDir::homePath()
    );
    if (targetDir.isEmpty()) {
       QMessageBox::information(nullptr, tr("提示"), tr("未选择目录，操作已取消"));
       return;
    }

    CustomFileSystemModel *fmodel = static_cast<CustomFileSystemModel*>(m_treeView->model());
    m_treeView->setRootIndex(fmodel->index(targetDir));
    golbalState.workPath = targetDir;
    golbalState.editPath = targetDir;
    m_listView->slot_loadImagesFromDirectory(targetDir);
    m_workPathShow->setText(golbalState.editPath);
}

void MainWindow::slot_act_closeDirectory()
{
    CustomFileSystemModel *fmodel = static_cast<CustomFileSystemModel*>(m_treeView->model());
    m_treeView->setRootIndex(fmodel->index(""));
}

void MainWindow::slot_act_manage_mkdir()
{
    QString basePath = golbalState.editPath;
    // 1. 获取用户输入的新文件夹名称
    bool ok;
    QString folderName = QInputDialog::getText(nullptr,
                                             "创建新文件夹",
                                             "请输入文件夹名称:",
                                             QLineEdit::Normal,
                                             "",
                                             &ok);

    // 用户点击取消
    if (!ok || folderName.isEmpty()) {
        return ;
    }

    // 2. 构造完整路径
    QDir baseDir(basePath);
    QString fullPath = baseDir.filePath(folderName);

    // 3. 尝试创建目录
    if (baseDir.mkdir(folderName)) {
        statusBar()->showMessage(QString(tr("文件夹创建成功:\n%1")).arg(fullPath));
    } else {
        statusBar()->showMessage(QString(tr("无法创建文件夹:\n%1")).arg(fullPath));
    }
}

void MainWindow::slot_act_manage_delete()
{
    // 获取所有选中的索引
    QModelIndexList selectedIndexes = m_listView->selectionModel()->selectedIndexes();
    // 提取选中项的数据
    QStringList selectedItems;
    for (const QModelIndex &index : selectedIndexes) {
        if (index.isValid()) {
            selectedItems << index.data(Qt::UserRole+1).toString(); // 获取显示文本
        }
    }
    // 提取行号并去重
    QList<int> rows;
    for (const QModelIndex &index : selectedIndexes) {
        if (index.isValid()) {
            int row = index.row();
            if (!rows.contains(row)) {
                rows.append(row);
            }
        }
    }
    // 按逆序排序（从大到小）
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    // 删除行
    for (int row : rows) {
        m_listView->model()->removeRow(row); // QStandardItemModel 直接删除行
    }
    // 创建进度条对话框
    QProgressDialog progressDialog;
    progressDialog.setWindowTitle(tr("处理进度"));
    progressDialog.setLabelText(tr("正在批量处理图片..."));
    progressDialog.setCancelButton(nullptr);
    progressDialog.setRange(0, selectedIndexes.size());
    progressDialog.setModal(true); // 设置为模态对话框，阻塞用户其他操作
    progressDialog.show();
    int i=0;
    //本地目录删除文件
    for (const QString &filePath : selectedItems) {
        QFileInfo fileInfo(filePath);
        QString pathName = fileInfo.path() + "/" + fileInfo.baseName() + ".path";
        QFile::remove(filePath);
        QFile::remove(pathName);

        progressDialog.setValue(i);
        progressDialog.setLabelText(QString(tr("正在处理：%1/%2")).arg(i+1).arg(selectedIndexes.size()));
        QApplication::processEvents();
        i++;
    }
    progressDialog.setValue(selectedIndexes.size());

}

void MainWindow::slot_act_manage_rename()
{
    // 获取所有选中的索引
    QModelIndexList selectedIndexes = m_listView->selectionModel()->selectedIndexes();
    if(selectedIndexes.size()==0) return;
    QDialog dialog;
    QFormLayout *formLay = new QFormLayout;
    QLineEdit *lineEdit = new QLineEdit("#####");
    QSpinBox *spinbox = new QSpinBox;spinbox->setRange(1,1000000);
    QPushButton *bt = new QPushButton(tr("确定"));
    formLay->addRow(tr("命名格式"),lineEdit);
    formLay->addRow(tr("起始序号"),spinbox);
    formLay->addRow(bt);
    dialog.setLayout(formLay);
    connect(bt,&QPushButton::clicked,&dialog,&QDialog::accept);
    if(dialog.exec() != QDialog::Accepted) return;

    QString fomatStr = lineEdit->text();
    int wdnum = fomatStr.count("#");
    int startnum = spinbox->value();

    QStringList psstr = fomatStr.split(".");
    QString preStr="";
    QString surStr="";
    if(psstr.size()>=1)  preStr = psstr[0];
    if(psstr.size()>=2)  surStr = psstr[1];
    preStr = preStr.replace("#","");

    // 提取选中项的数据
    // 创建进度条对话框
    QProgressDialog progressDialog;
    progressDialog.setWindowTitle(tr("处理进度"));
    progressDialog.setLabelText(tr("正在批量处理图片..."));
    progressDialog.setCancelButton(nullptr);
    progressDialog.setRange(0, selectedIndexes.size());
    progressDialog.setModal(true); // 设置为模态对话框，阻塞用户其他操作
    progressDialog.show();
    int i=0;
    for (const QModelIndex &index : selectedIndexes) {
        if (index.isValid()) {
            QString oldFileName = index.data(Qt::UserRole+1).toString(); // 旧文件名称
            QFileInfo fileInfo(oldFileName);
            QString newBaseName = QString("%1").arg(startnum, wdnum, 10, QChar('0'));
            QString dotName = surStr.isEmpty()?fileInfo.suffix():surStr;
            newBaseName = preStr + newBaseName + "." + dotName;
            QString newFileName = fileInfo.path() + "/" + newBaseName;

            // 重命名文件
            if (QFile::rename(oldFileName, newFileName)) {
                // 更新模型的 UserRole+1 数据
                m_listView->model()->setData(index, newFileName, Qt::UserRole + 1);
                m_listView->model()->setData(index, newBaseName, Qt::DisplayRole);

                QFileInfo fileInfo(oldFileName);
                QString oldPathName = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".path";
                QFileInfo fileInfo1(newFileName);
                QString newPathName = fileInfo1.path() + "/" +fileInfo1.completeBaseName() + ".path";
                //qDebug()<<oldPathName<<newPathName;
                QFile::rename(oldPathName, newPathName);

                startnum++;
            }

            progressDialog.setValue(i);
            progressDialog.setLabelText(QString(tr("正在处理：%1/%2")).arg(i+1).arg(selectedIndexes.size()));
            QApplication::processEvents();
            i++;

        }
    }

    progressDialog.setValue(selectedIndexes.size());
}

void MainWindow::slot_act_manage_rotate90r()
{
    fun_rotate_select_image(90);
}

void MainWindow::slot_act_rotate90l()
{
    fun_rotate_select_image(-90);
}

void MainWindow::slot_act_manage_copy()
{
    // 1. 让用户选择目标目录
    QString targetDir = QFileDialog::getExistingDirectory(
       nullptr,
       tr("选择目标目录"),
       QDir::homePath()
    );
    if (targetDir.isEmpty()) {
       QMessageBox::information(nullptr, tr("提示"), tr("未选择目录，操作已取消"));
       return;
    }

    // 2. 获取选中的项
    QModelIndexList selectedIndexes = m_listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
       QMessageBox::warning(nullptr, tr("警告"), tr("请先选中要复制的图片"));
       return;
    }
    // 3. 遍历选中项并复制文件
    // 创建进度条对话框
    QProgressDialog progressDialog;
    progressDialog.setWindowTitle(tr("处理进度"));
    progressDialog.setLabelText(tr("正在批量处理图片..."));
    progressDialog.setCancelButton(nullptr);
    progressDialog.setRange(0, selectedIndexes.size());
    progressDialog.setModal(true); // 设置为模态对话框，阻塞用户其他操作
    progressDialog.show();
    int i=0;
    int successCount = 0;
    int failCount = 0;
    QStringList fileFileNames;
    for (const QModelIndex &index : selectedIndexes) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(QString(tr("正在处理：%1/%2")).arg(i+1).arg(selectedIndexes.size()));
        QApplication::processEvents();
        i++;
        if (!index.isValid()) continue;
        // 3.1 获取文件路径
        QString sourcePath = index.data(Qt::UserRole + 1).toString();
        QFile sourceFile(sourcePath);

        // 检查文件是否存在
        if (!sourceFile.exists()) {
            fileFileNames<<sourcePath;
            failCount++;
            continue;
        }

        // 3.2 构造目标路径
        QFileInfo fileInfo(sourcePath);
        QString destPath = QDir(targetDir).filePath(fileInfo.fileName());

        // 3.3 检查文件是否已存在
        if (QFile::exists(destPath)) {
            // 如果目标文件已存在，可以选择覆盖或跳过（这里直接覆盖）
            if (!QFile::remove(destPath)) {
                fileFileNames<<sourcePath;
                failCount++;
                continue;
            }
        }

        // 3.4 执行复制
        if (sourceFile.copy(destPath)) {
            successCount++;
        } else {
            fileFileNames<<sourcePath;
            failCount++;
        }
    }

    progressDialog.setValue(selectedIndexes.size());
    // 4. 显示结果
    if(failCount>0)
    {
        QString message = QString(tr("操作完成：成功复制 %1 个文件，失败 %2 个")).arg(successCount).arg(failCount);
        message+="\n";
        message+=fileFileNames.join("\n");
        QMessageBox::information(nullptr, tr("结果"), message);
    }
}

void MainWindow::slot_act_manage_move()
{
    // 1. 让用户选择目标目录
    QString targetDir = QFileDialog::getExistingDirectory(
       nullptr,
       tr("选择目标目录"),
       QDir::homePath()
    );
    if (targetDir.isEmpty()) {
       QMessageBox::information(nullptr, tr("提示"), tr("未选择目录，操作已取消"));
       return;
    }

    // 2. 获取选中的项
    QModelIndexList selectedIndexes = m_listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
       QMessageBox::warning(nullptr, tr("警告"), tr("请先选中要复制的图片"));
       return;
    }
    // 3. 遍历选中项并复制文件
    // 创建进度条对话框
    QProgressDialog progressDialog;
    progressDialog.setWindowTitle(tr("处理进度"));
    progressDialog.setLabelText(tr("正在批量处理图片..."));
    progressDialog.setCancelButton(nullptr);
    progressDialog.setRange(0, selectedIndexes.size());
    progressDialog.setModal(true); // 设置为模态对话框，阻塞用户其他操作
    progressDialog.show();
    int i=0;
    int successCount = 0;
    int failCount = 0;
    QStringList fileFileNames;
    QModelIndexList sucessIndexes;
    for (const QModelIndex &index : selectedIndexes) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(QString(tr("正在处理：%1/%2")).arg(i+1).arg(selectedIndexes.size()));
        QApplication::processEvents();
        i++;
        if (!index.isValid()) continue;
        // 3.1 获取文件路径
        QString sourcePath = index.data(Qt::UserRole + 1).toString();
        QFile sourceFile(sourcePath);

        // 检查文件是否存在
        if (!sourceFile.exists()) {
            fileFileNames<<sourcePath;
            failCount++;
            continue;
        }

        // 3.2 构造目标路径
        QFileInfo fileInfo(sourcePath);
        QString destPath = QDir(targetDir).filePath(fileInfo.fileName());

        // 3.3 检查文件是否已存在
        if (QFile::exists(destPath)) {
            // 如果目标文件已存在，可以选择覆盖或跳过（这里直接覆盖）
            if (!QFile::remove(destPath)) {
                fileFileNames<<sourcePath;
                failCount++;
                continue;
            }
        }

        // 3.4 执行复制
        if (sourceFile.copy(destPath)) {
            successCount++;
            QFile::remove(sourcePath);//删除源文件
            sucessIndexes<<index;
        } else {
            fileFileNames<<sourcePath;
            failCount++;
        }
    }

    progressDialog.setValue(selectedIndexes.size());
    // 提取所有需要删除的行号（去重）
    QList<int> rowsToRemove;
    for (const QModelIndex& index : sucessIndexes) {
        if (index.isValid()) {
            rowsToRemove << index.row();
        }
    }

    // 去重并排序（从大到小删除，避免行号变化）
    std::sort(rowsToRemove.begin(), rowsToRemove.end(), std::greater<int>());
    rowsToRemove.erase(std::unique(rowsToRemove.begin(), rowsToRemove.end()), rowsToRemove.end());

    // 批量删除行
    for (int row : rowsToRemove) {
        m_listView->model()->removeRow(row);
    }
    // 4. 显示结果
    if(failCount>0)
    {
        QString message = QString(tr("操作完成：成功复制 %1 个文件，失败 %2 个")).arg(successCount).arg(failCount);
        message+="\n";
        message+=fileFileNames.join("\n");
        QMessageBox::information(nullptr, tr("结果"), message);
    }
}

void MainWindow::slot_act_manage_cut()
{
#define SIZE_SVWIDTH 400
#define SIZE_SVHEIGHT 400
    QDialog dialog;
    dialog.setWindowTitle(tr("剪裁设置"));
    QGraphicsScene *sence = new QGraphicsScene;
    sence->setSceneRect(QRectF(0,0,SIZE_SVWIDTH,SIZE_SVHEIGHT));
    //sence->setBackgroundBrush(QBrush(QColor(Qt::black)));
    QGraphicsPixmapItem *pixItem = new QGraphicsPixmapItem;
    QPixmap pixmap(SIZE_SVWIDTH, SIZE_SVHEIGHT);
    pixmap.fill(Qt::black);  // 填充黑色
    pixItem->setPixmap(pixmap);
    sence->addItem(pixItem);
    QGraphicsView *view = new QGraphicsView;
    view->setScene(sence);
    QGraphicsRectItem *rectItem = new QGraphicsRectItem;
    rectItem->setPen(QPen(QColor(Qt::green)));
    sence->addItem(rectItem);
    QGraphicsEllipseItem  *ellItem = new QGraphicsEllipseItem;ellItem->setVisible(false);
    ellItem->setRect(-5,-5,10,10);
    ellItem->setBrush(QBrush(QColor(Qt::green)));
    ellItem->setPen(QPen(QColor(Qt::green)));
    sence->addItem(ellItem);
    QHBoxLayout *hlyout = new QHBoxLayout;
    QFormLayout  *flyout = new QFormLayout;
    QDoubleSpinBox *dbboxX = new QDoubleSpinBox;dbboxX->setSuffix("%");dbboxX->setDecimals(3);
    QDoubleSpinBox *dbboxY = new QDoubleSpinBox;dbboxY->setSuffix("%");dbboxY->setDecimals(3);
    QDoubleSpinBox *dbboxW = new QDoubleSpinBox;dbboxW->setSuffix("%");dbboxW->setDecimals(3);
    QDoubleSpinBox *dbboxH = new QDoubleSpinBox;dbboxH->setSuffix("%");dbboxH->setDecimals(3);
    QCheckBox *chekbox = new QCheckBox(tr("剪裁后缩放"));
    QSpinBox *dbscalW= new QSpinBox;dbscalW->setSuffix("pix");dbscalW->setRange(0,9999);dbscalW->setValue(512);
    QSpinBox *dbscalH = new QSpinBox;dbscalH->setSuffix("pix");dbscalH->setRange(0,9999);dbscalH->setValue(512);
    flyout->addRow(tr("剪裁位置X"),dbboxX);
    flyout->addRow(tr("剪裁位置Y"),dbboxY);
    flyout->addRow(tr("剪裁宽度W"),dbboxW);
    flyout->addRow(tr("剪裁高度H"),dbboxH);

    QCheckBox *chekboxMAX = new QCheckBox(tr("目标点最大内接正方形剪裁"));
    flyout->addRow(chekboxMAX);
    connect(chekboxMAX,&QCheckBox::checkStateChanged,[ellItem,rectItem,dbboxW,dbboxH](int sta){
        if(sta){
            dbboxW->setVisible(false);
            dbboxH->setVisible(false);
            ellItem->setVisible(true);
            rectItem->setVisible(false);
        }
        else{
            dbboxW->setVisible(true);
            dbboxH->setVisible(true);
            ellItem->setVisible(false);
            rectItem->setVisible(true);
        }
    });
    QPushButton *bt = new QPushButton(tr("确定剪裁"));
    connect(bt,&QPushButton::clicked,&dialog,&QDialog::accept);

    // 添加分割线（水平线）
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);  // 水平线
    line->setFrameShadow(QFrame::Sunken); // 凹陷效果
    flyout->addRow(line); // 单独占一行
    flyout->addRow(chekbox);
    flyout->addRow(tr("剪裁后缩到宽度"),dbscalW);
    flyout->addRow(tr("剪裁缩放到高度"),dbscalH);
    QFrame *line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);  // 水平线
    line1->setFrameShadow(QFrame::Sunken); // 凹陷效果
    flyout->addRow(line1); // 单独占一行
    flyout->addRow(bt);
    hlyout->addWidget(view,4);
    hlyout->addLayout(flyout,1);


    connect(dbboxX, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, [ellItem,rectItem,dbboxW](double value) {
        QRectF rectf = rectItem->rect();
        rectf.setX(SIZE_SVWIDTH*value*0.01);
        rectItem->setRect(rectf);
        dbboxW->setValue(100*rectf.width()/SIZE_SVWIDTH);
        ellItem->setPos(QPointF(rectf.x(),rectf.y()));
    });
    connect(dbboxY, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, [ellItem,rectItem,dbboxH](double value) {
        QRectF rectf = rectItem->rect();
        rectf.setY(SIZE_SVHEIGHT*value*0.01);
        rectItem->setRect(rectf);
        dbboxH->setValue(100*rectf.height()/SIZE_SVHEIGHT);
        ellItem->setPos(QPointF(rectf.x(),rectf.y()));
    });
    connect(dbboxW, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, [dbboxX,dbboxW,rectItem](double value) {
        QRectF rectf = rectItem->rect();
        if((value + dbboxX->value())>=100)
            dbboxW->setValue(100 -  dbboxX->value());
        rectf.setWidth(SIZE_SVWIDTH*dbboxW->value()*0.01);
        rectItem->setRect(rectf);
    });
    connect(dbboxH, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, [dbboxY,dbboxH,rectItem](double value) {
        QRectF rectf = rectItem->rect();
        if((value + dbboxY->value())>=100)
            dbboxH->setValue(100 -  dbboxY->value());
        rectf.setHeight(SIZE_SVHEIGHT*dbboxH->value()*0.01);
        rectItem->setRect(rectf);
    });

    dialog.setLayout(hlyout);
    dbboxX->setValue(20);
    dbboxY->setValue(20);
    dbboxW->setValue(60);
    dbboxH->setValue(60);
    if(dialog.exec() == QDialog::Accepted)
    {
        int cutmode = chekboxMAX->isChecked()?1:0;
        qreal x = dbboxX->value()*0.01;
        qreal y = dbboxY->value()*0.01;
        qreal w = dbboxW->value()*0.01;
        qreal h = dbboxH->value()*0.01;
        bool doscal = chekboxMAX->isChecked();
        int scalW = dbscalW->value();
        int scalH = dbscalH->value();
        pixmapCutScal(cutmode,x,y,w,h,doscal,scalW,scalH);
    }

}
// 计算矩形的旋转角度
double calculateRotation(const QPointF& p1, const QPointF& p2) {
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return std::atan2(dy, dx) * 180.0 / M_PI;
}
double calculateDis(const QPointF& p1, const QPointF& p2)
{
    // 计算两点之间的距离
    qreal dx = p1.x() - p2.x();
    qreal dy = p1.y() - p2.y();
    double distance = std::sqrt(dx * dx + dy * dy);
    return distance;
}

void MainWindow::slot_act_editPixMap(QString filePath)
{
    m_sence->loadPiamap(filePath);

    QString pfname = filePath;
    //加载图片
    if(pfname=="") return;
    m_listWidgeLabels->clear();
    m_mapItemInfo.clear();
    //查看标注文件是否存在
    QFileInfo fileInfo(pfname);
    QString labelName = fileInfo.path() + "/" + fileInfo.baseName() + ".path";
    qreal w = m_sence->width();
    qreal h = m_sence->height();
    QFile ifileTxt(labelName);
    if(ifileTxt.exists())
    {
        ifileTxt.open(QIODevice::ReadOnly);
        QString strAll = ifileTxt.readAll();
        QStringList strlst = strAll.split("\n");

        for(int x=0;x!=strlst.size();x++)
        {
            QStringList paramLst = strlst[x].split(" ");
            QString infoType = paramLst.takeAt(0);
            if(infoType=="rect")
            {
                if(paramLst.size()<9){
                    break;
                }
                int type = paramLst[0].toInt();
                QPointF p1 = QPointF(paramLst[1].toDouble()*w,paramLst[2].toDouble()*h);
                QPointF p2 = QPointF(paramLst[3].toDouble()*w,paramLst[4].toDouble()*h);
                //QPointF p3 = QPointF(paramLst[5].toDouble()*w,paramLst[6].toDouble()*h);
                QPointF p4 = QPointF(paramLst[7].toDouble()*w,paramLst[8].toDouble()*h);
                qreal wl = calculateDis(p1,p2);
                qreal hl = calculateDis(p2,p4);
                QRectF rectf(0,0,wl,hl);
                // 计算矩形的中心和旋转角度
                double rotation = calculateRotation(p1, p2);
                LQGraphicsRectItem *currentRect = new LQGraphicsRectItem;
                m_sence->addItem(currentRect);
                currentRect->setRect(rectf);
                // 设置矩形的位置和旋转角度
                currentRect->setTransformOriginPoint(0,0);
                currentRect->setRotation(rotation);
                currentRect->setPos(p1);
                golbalState.curIndex = type;
                m_sence->rect_ItemClassChange(currentRect,false);
                currentRect->setSelected(false);
                m_mapItemInfo[strlst[x]] = currentRect;
                m_listWidgeLabels->addItem(strlst[x]);
            }
            else if(infoType=="path")
            {
                int type = paramLst.takeAt(0).toInt();
                QVector<QPointF> points;
                for(int i=0;i!=paramLst.size()/2;i++){
                     QPointF p1 = QPointF(paramLst[i*2].toDouble()*w,paramLst[i*2+1].toDouble()*h);
                     points.append(p1);
                }

                if(points.size()<3)  continue;

                LQGraphicsPathItem *pathItem = new LQGraphicsPathItem();
                m_sence->addItem(pathItem);
                for(int i=0;i!=points.size();i++)
                {
                    if(i==0){
                        pathItem->startPathPoint(points[i]);
                    }
                    else if(i==(points.size()-1)){
                        pathItem->appendPathPoint(points[i]);
                        pathItem->endPathPoint();
                    }
                    else{
                        pathItem->appendPathPoint(points[i]);
                    }
                }

                golbalState.curIndex = type;
                m_sence->polygon_ItemClassChange(pathItem,false);
                m_mapItemInfo[strlst[x]] = pathItem;
                m_listWidgeLabels->addItem(strlst[x]);
            }
        }

        ifileTxt.close();

    }

    //显示图片
    QPixmap pixmapLabel = captureScene(m_sence);
    if (!pixmapLabel.isNull()) {
        // 将缩小后的图片设置到 QLabel 上
        m_slLabel->setPixmap(pixmapLabel);
    }
}

void MainWindow::slot_act_rect()
{
    golbalState.labelMode = LABEL_RECT_MODE;
}

void MainWindow::slot_act_polygon()
{
    golbalState.labelMode = LABEL_POLYGON_MODE;
}

void MainWindow::slot_act_rectRoateR2()
{
    m_sence->rect_item_roate(5);
}

void MainWindow::slot_act_rectRoateL2()
{
    m_sence->rect_item_roate(-5);
}

void MainWindow::slot_act_rectRoateR01()
{
    m_sence->rect_item_roate(0.1);
}

void MainWindow::slot_act_rectRoateL01()
{
    m_sence->rect_item_roate(-0.1);
}

void MainWindow::slot_act_rectWidthAdd()
{
    m_sence->rect_item_widthAddPlus(2);
}

void MainWindow::slot_act_rectWidthSub()
{
    m_sence->rect_item_widthAddPlus(-2);
}

void MainWindow::slot_act_rectHeightAdd()
{
    m_sence->rect_item_heightAddPlus(2);
}

void MainWindow::slot_act_rectHeightSub()
{
    m_sence->rect_item_heightAddPlus(-2);
}

void MainWindow::slot_act_rectCopy()
{
    m_sence->rect_item_copy();
}

void MainWindow::slot_act_rectPaste()
{
    m_sence->rect_item_paste();
}

void MainWindow::slot_act_selectClass()
{
    m_sence->rect_item_change();
    m_sence->polygon_item_change();
}

void MainWindow::slot_act_setting()
{
    //参数修改
    golbalState.readSettingIni();
    GlobalSettingsDialog dialog(golbalState);
    if (dialog.exec() == QDialog::Accepted) {
        // 用户点击确定，数据已自动保存
        qDebug() << "当前类别:" << golbalState.curCsName();
    } else {
        // 用户取消，数据未修改
    }

}


void MainWindow::slot_act_viewZoonIn()
{
    m_view->scale(0.9,0.9);
}

void MainWindow::slot_act_viewZoonOut()
{
      m_view->scale(1.1,1.1);
}

void MainWindow::slot_act_deleteItem()
{
    this->m_sence->rectPloygon_item_delete();
}

void MainWindow::slot_act_saveLableInfo()
{
    if(!m_listView->currentIndex().isValid())  return;
    //获取保存文件名并创建文件
    QString srcFileName = m_listView->currentIndex().data(Qt::UserRole+1).toString();
    QFileInfo fileInfo(srcFileName);
    QString pathName = fileInfo.path() + "/" + fileInfo.baseName() + ".path";
    QFile outfile(pathName);
    if(!outfile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this,tr("提示"),tr("文件被暂用"));
        return;
    }
    //获取场景中的图元
    m_mapItemInfo.clear();
    m_listWidgeLabels->clear();
    QList<QGraphicsItem *> sitems = m_sence->items();
    QRectF sRect = m_sence->sceneRect();
    qreal w = sRect.width();
    qreal h = sRect.height();

    for(int i=0;i!=sitems.size();i++)
    {
        if(sitems[i]->type() == LQGraphicsRectItem::Type)
        {
            LQGraphicsRectItem *rectItem = static_cast<LQGraphicsRectItem*>(sitems[i]);

            QRectF rectf = rectItem->rect();
            QPointF p1 = rectf.topLeft();
            QPointF p2 = rectf.topRight();
            QPointF p3 = rectf.bottomLeft();
            QPointF p4 = rectf.bottomRight();
            p1 = rectItem->mapToScene(p1);
            p2 = rectItem->mapToScene(p2);
            p3 = rectItem->mapToScene(p3);
            p4 = rectItem->mapToScene(p4);
            int type = rectItem->data(Qt::UserRole+8).toInt();
            QString strwt;
            strwt = strwt.asprintf("rect %d %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n",type,p1.x()/w,p1.y()/h
                                                                             ,p2.x()/w,p2.y()/h
                                                                             ,p3.x()/w,p3.y()/h
                                                                             ,p4.x()/w,p4.y()/h);
            outfile.write(strwt.toLocal8Bit());
            QString rectInfos = strwt.replace("\n","");
            m_mapItemInfo[rectInfos] = rectItem;
            m_listWidgeLabels->addItem(rectInfos);
        }
        else if(sitems[i]->type() == LQGraphicsPathItem::Type)
        {
            LQGraphicsPathItem *pathItem = static_cast<LQGraphicsPathItem*>(sitems[i]);
            QList<LQGraphicsEllipseItem *>  childs = pathItem->childEllipsePoints();
            int type = pathItem->data(Qt::UserRole+8).toInt();

            QString strwt;
            strwt += strwt.asprintf("path %d",type);
            QVector<QPointF> pointsf;
            foreach(auto one,childs)
            {
                QPointF ps = one->scenePos();
                strwt += strwt.asprintf(" %.6f %.6f",ps.x()/w,ps.y()/h);
                pointsf.append(ps);
            }
            strwt += "\n";
            outfile.write(strwt.toLocal8Bit());

            QString pathInfos = strwt.replace("\n","");
            m_mapItemInfo[pathInfos] = pathItem;
            m_listWidgeLabels->addItem(pathInfos);
        }

    }

    //显示图片
    QPixmap pixmapLabel = captureScene(m_sence);
    if (!pixmapLabel.isNull()) {
        // 将缩小后的图片设置到 QLabel 上
        m_slLabel->setPixmap(pixmapLabel);
    }
    outfile.close();
    statusBar()->showMessage(tr("保存信息文件->") + pathName);
}

void MainWindow::slot_act_prevPix()
{
    QModelIndex current = m_listView->currentIndex();
    QAbstractItemModel *model = m_listView->model();
    if (!model || model->rowCount() == 0) return;

    int newRow = current.isValid() ? (current.row() - 1 + model->rowCount()) % model->rowCount()
                                   : model->rowCount() - 1;
    QModelIndex newIndex = model->index(newRow, 0);
    m_listView->setCurrentIndex(newIndex);
    m_listView->scrollTo(newIndex);
}

void MainWindow::slot_act_nextPix()
{
    // 切换到下一个项
    QModelIndex current = m_listView->currentIndex();
    QAbstractItemModel *model = m_listView->model();
    if (!model || model->rowCount() == 0) return;

    int newRow = current.isValid() ? (current.row() + 1) % model->rowCount() : 0;
    QModelIndex newIndex = model->index(newRow, 0);
    m_listView->setCurrentIndex(newIndex);
    m_listView->scrollTo(newIndex);
}

void MainWindow::slot_act_addEll()
{
    m_sence->polygon_item_insertPoint();
}


void MainWindow::slot_act_modeMange()
{
    m_listView->setHorizontalFlowMode();
    m_dockFileTree->show();
    resizeDocks({m_dockFileTree, m_dockImageList}, {this->width()/8, this->width()*29/80}, Qt::Horizontal);
    this->updateGeometry();
    QApplication::processEvents();
    m_sence->setScenFill();
}

void MainWindow::slot_act_modeLabel()
{
    m_dockFileTree->hide();
    m_listView->setVerticalListMode();
    resizeDocks({m_dockImageList}, {this->width()/8}, Qt::Horizontal);
    this->updateGeometry();
    QApplication::processEvents();
    m_sence->setScenFill();
}

void MainWindow::slot_act_dataCOCO()
{

}

void MainWindow::slot_act_dataVOC()
{
    QString inputDir;
    QString outputDir;
    QDialog dialog(this);
    dialog.setWindowTitle(tr("选择目录"));

    // 主布局
    QFormLayout layout(&dialog);
    QComboBox *combox = new QComboBox;
    combox->addItems({tr("目标检测"),tr("语义分割")});
    // 第一行：输入目录
    QLineEdit inputEdit(m_workPathShow->text());
    inputEdit.setReadOnly(true);
    QPushButton inputBtn(tr("选择..."));
    QObject::connect(&inputBtn, &QPushButton::clicked, [&]() {
        QString dir = QFileDialog::getExistingDirectory(&dialog, tr("选择输入目录"), inputEdit.text().isEmpty() ? QDir::homePath() : inputEdit.text());
        if (!dir.isEmpty()) {
            inputEdit.setText(QDir::toNativeSeparators(dir));
        }
    });

    // 第二行：输出目录
    QLineEdit outputEdit(m_workPathShow->text() + "/voc");
    //outputEdit.setReadOnly(true);
    QPushButton outputBtn(tr("选择..."));
    QObject::connect(&outputBtn, &QPushButton::clicked, [&]() {
        QString dir = QFileDialog::getExistingDirectory(&dialog, tr("选择输出目录"), outputEdit.text().isEmpty() ? QDir::homePath() : outputEdit.text());
        if (!dir.isEmpty()) {
            outputEdit.setText(QDir::toNativeSeparators(dir));
        }
    });

    // 创建带按钮的行控件
    auto createRow = [](QLineEdit *edit, QPushButton *btn) {
        QWidget *container = new QWidget;
        QHBoxLayout *hLayout = new QHBoxLayout(container);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->addWidget(edit);
        hLayout->addWidget(btn);
        return container;
    };

    QHBoxLayout *hlayout = new QHBoxLayout;hlayout->setAlignment(Qt::AlignLeft);
    QSpinBox *spinbox1 = new QSpinBox();spinbox1->setValue(10);
    QSpinBox *spinbox2 = new QSpinBox();spinbox2->setValue(2);
    QSpinBox *spinbox3 = new QSpinBox();spinbox3->setValue(1);
    hlayout->addWidget(new QLabel(tr("训练集比例:")));
    hlayout->addWidget(spinbox1);
    hlayout->addWidget(new QLabel(tr("验证集比例:")));
    hlayout->addWidget(spinbox2);
    hlayout->addWidget(new QLabel(tr("测试集比例:")));
    hlayout->addWidget(spinbox3);
    // 添加到布局
    layout.addRow(tr("模型格式"), combox);
    layout.addRow(tr("输入目录:"), createRow(&inputEdit, &inputBtn));
    layout.addRow(tr("输出目录:"), createRow(&outputEdit, &outputBtn));
    layout.addRow(hlayout);

    // 确认按钮
    QPushButton okBtn(tr("确定"));
    layout.addRow(&okBtn);
    QObject::connect(&okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.resize(800,150);
    if (dialog.exec() == QDialog::Accepted) {
        inputDir = inputEdit.text();
        outputDir = outputEdit.text();
        if(outputDir.isEmpty()) return;
        VOCExport::ExportMode mode = VOCExport::ExportMode(combox->currentIndex());
        VOCExport yoloExporter;
        VOCExport converter;
        converter.setExportParam(mode,inputDir, outputDir, golbalState.classNames,spinbox1->value(),spinbox2->value(),spinbox3->value());
        converter.exportDataset();
        this->statusBar()->showMessage(tr("VOC数据集完成->")+outputDir);
    }
}

void MainWindow::slot_act_dataYOLO()
{
    QString inputDir;
    QString outputDir;
    QDialog dialog(this);
    dialog.setWindowTitle(tr("选择目录"));

    // 主布局
    QFormLayout layout(&dialog);

    QComboBox *combox = new QComboBox;
    combox->addItems({tr("目标检测"),tr("旋转矩形"),tr("实例分割")});

    // 第一行：输入目录
    QLineEdit inputEdit(m_workPathShow->text());
    inputEdit.setReadOnly(true);
    QPushButton inputBtn(tr("选择..."));
    QObject::connect(&inputBtn, &QPushButton::clicked, [&]() {
        QString dir = QFileDialog::getExistingDirectory(&dialog, tr("选择输入目录"), inputEdit.text().isEmpty() ? QDir::homePath() : inputEdit.text());
        if (!dir.isEmpty()) {
            inputEdit.setText(QDir::toNativeSeparators(dir));
        }
    });

    // 第二行：输出目录
    QLineEdit outputEdit(m_workPathShow->text()+"/dataset/");
    //outputEdit.setReadOnly(true);
    QPushButton outputBtn(tr("选择..."));
    QObject::connect(&outputBtn, &QPushButton::clicked, [&]() {
        QString dir = QFileDialog::getExistingDirectory(&dialog, tr("选择输出目录"), outputEdit.text().isEmpty() ? QDir::homePath() : outputEdit.text());
        if (!dir.isEmpty()) {
            outputEdit.setText(QDir::toNativeSeparators(dir));
        }
    });
    QHBoxLayout *hlayout = new QHBoxLayout;hlayout->setAlignment(Qt::AlignLeft);
    QSpinBox *spinboxTrain= new QSpinBox();spinboxTrain->setValue(10);
    QSpinBox *spinboxVal = new QSpinBox();spinboxVal->setValue(2);
    QSpinBox *spinboxTest = new QSpinBox();spinboxTest->setValue(1);
    hlayout->addWidget(new QLabel(tr("训练集比例:")));
    hlayout->addWidget(spinboxTrain);
    hlayout->addWidget(new QLabel(tr("验证集比例:")));
    hlayout->addWidget(spinboxVal);
    hlayout->addWidget(new QLabel(tr("测试集比例:")));
    hlayout->addWidget(spinboxTest);

    // 创建带按钮的行控件
    auto createRow = [](QLineEdit *edit, QPushButton *btn) {
        QWidget *container = new QWidget;
        QHBoxLayout *hLayout = new QHBoxLayout(container);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->addWidget(edit);
        hLayout->addWidget(btn);
        return container;
    };

    // 添加到布局
    layout.addRow(tr("模型格式"), combox);
    layout.addRow(tr("输入目录:"), createRow(&inputEdit, &inputBtn));
    layout.addRow(tr("输出目录:"), createRow(&outputEdit, &outputBtn));
    layout.addRow(hlayout);

    // 确认按钮
    QPushButton okBtn(tr("确定"));
    layout.addRow(&okBtn);
    QObject::connect(&okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.resize(800,150);
    if (dialog.exec() == QDialog::Accepted) {
        inputDir = inputEdit.text();
        outputDir = outputEdit.text();
        if(outputDir.isEmpty()) return;
        YOLOExporter::ExportMode mode = YOLOExporter::ExportMode(combox->currentIndex());
        YOLOExporter yoloExporter;
        yoloExporter.setExportParam(mode,inputDir,outputDir,golbalState.classNames,spinboxTrain->value(),spinboxVal->value(),spinboxTest->value());
        yoloExporter.exportDataset();
        this->statusBar()->showMessage(tr("VOC数据集完成->")+outputDir);
    }
}

void MainWindow::slot_postion(QPointF ps)
{
//    qreal w = m_sence->width();
//    qreal h = m_sence->height();
    QString strShow = QString("X: %1pix Y: %2pix").arg(int(ps.x()), 4, 10, QChar('0'))
                                    .arg(int(ps.y()), 4, 10, QChar('0'));

    m_labelMouse->setText(strShow);
}

void MainWindow::slot_changeTheme(QString themeStr)
{
    if(themeStr=="dark")
        qApp->setStyleSheet(getQSSFile(":/styles/dark.qss"));
    else
        qApp->setStyleSheet("");
}

void MainWindow::slot_changeLang(QString langStr)
{
    if(langStr == "zh")
    {
        if (m_translator.load(":/translations/LabelPixmap_zh_CN.qm")) {
            qApp->installTranslator(&m_translator);
        }
    }
    else if(langStr == "en")
    {
        if (m_translator.load(":/translations/LabelPixmap_en.qm")) {
            qApp->installTranslator(&m_translator);
        }
    }
}

void MainWindow::fun_rotate_select_image(int arg)
{
    // 获取所有选中的索引
    QModelIndexList selectedIndexes = m_listView->selectionModel()->selectedIndexes();
    // 创建进度条对话框
    QProgressDialog progressDialog;
    progressDialog.setWindowTitle(tr("处理进度"));
    progressDialog.setLabelText(tr("正在批量处理图片..."));
    progressDialog.setCancelButtonText(tr("取消")); // 可选：添加取消按钮
    progressDialog.setRange(0, selectedIndexes.size());
    progressDialog.setModal(true); // 设置为模态对话框，阻塞用户其他操作
    progressDialog.show();

    // 提取选中项的数据
    QStringList selectedItems;
    int i=0;
    for (const QModelIndex &index : selectedIndexes) {
        if (index.isValid()) {
            QString fileName = index.data(Qt::UserRole+1).toString(); // 获取显示文本
            // 检查是否取消操作
            if (progressDialog.wasCanceled()) {
                break;
            }
            QImage image(fileName);
            if (!image.isNull()) {
                // 2. 创建旋转矩阵（顺时针旋转90度）
                QTransform transform;
                transform.rotate(arg); // 顺时针旋转90度
                // 3. 应用旋转（并自动调整尺寸）
                QImage rotatedImage = image.transformed(transform, Qt::SmoothTransformation);
                rotatedImage.save(fileName);
            }
            m_listView->reloadOneImage(index.row(),fileName);
            // 更新进度条和描述
            progressDialog.setValue(i);
            progressDialog.setLabelText(QString(tr("正在处理：%1/%2")).arg(i+1).arg(selectedIndexes.size()));
            QApplication::processEvents();
            i++;
        }
    }
    // 完成或取消后关闭对话框
    progressDialog.setValue(selectedIndexes.size());
}

void MainWindow::pixmapCutScal(int cutmode, qreal x, qreal y, qreal w, qreal h, bool doscal, int scalW, int scalH)
{
    // 1. 让用户选择目标目录
    QString targetDir = QFileDialog::getExistingDirectory(
       nullptr,
       tr("选择输出目录"),
       QDir::homePath()
    );
    if (targetDir.isEmpty()) {
       QMessageBox::information(nullptr, tr("提示"), tr("未选择目录，操作已取消"));
       return;
    }

    // 2. 获取选中的项
    QModelIndexList selectedIndexes = m_listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
       QMessageBox::warning(nullptr, tr("警告"), tr("请先选中要复制的图片"));
       return;
    }
    // 3. 遍历选中项并复制文件
    // 创建进度条对话框
    QProgressDialog progressDialog;
    progressDialog.setWindowTitle(tr("处理进度"));
    progressDialog.setLabelText(tr("正在批量处理图片..."));
    progressDialog.setCancelButton(nullptr);
    progressDialog.setRange(0, selectedIndexes.size());
    progressDialog.setModal(true); // 设置为模态对话框，阻塞用户其他操作
    progressDialog.show();
    int i=0;

    QStringList fileFileNames;
    for (const QModelIndex &index : selectedIndexes) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(QString(tr("正在处理：%1/%2")).arg(i+1).arg(selectedIndexes.size()));
        QApplication::processEvents();
        i++;
        if (!index.isValid()) continue;
        // 3.1 获取原始图片文件路径
        QString sourcePath = index.data(Qt::UserRole + 1).toString();
        QImage originalImage(sourcePath);
        int pwidth = originalImage.width();
        int pheigh = originalImage.height();
        // 3.2 构造目标路径
        QFileInfo fileInfo(sourcePath);
        QString destPath = QDir(targetDir).filePath("cut_" + fileInfo.fileName());

        //3.3剪裁
        QImage croppedImage;
        //qDebug()<<cutmode;
        if(cutmode)//正方形剪裁
        {
            int leftX = x*pwidth;
            int topY = y*pheigh;
            int rightX = pwidth - x*pwidth;
            int bottomY = pheigh - y*pheigh;
            int min = std::min({leftX, topY, rightX, bottomY});
            int squareX = leftX - min;
            int squareY = topY - min;
            int squareSide = min;
            QRect cropRect(squareX, squareY, squareSide, squareSide);
            croppedImage = originalImage.copy(cropRect);
        }
        else//比例剪裁
        {
            QRect cropRect(x*pwidth, y*pheigh, w*pwidth, h*pheigh);
            croppedImage = originalImage.copy(cropRect);
        }
        if(!croppedImage.isNull())
        {
            if(doscal)
            {
                croppedImage = croppedImage.scaled(scalW,scalH);
            }
            croppedImage.save(destPath);
        }

    }

    progressDialog.setValue(selectedIndexes.size());

}

