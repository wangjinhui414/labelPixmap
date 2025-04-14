#include "global_data.h"
#include <QColorDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QFrame>
#include <QSettings>
#include <QColor>
#include <QVector>


#define SETTING_FILE_NAME "setting.cfg"
TYPE_GOLBAL_DATA golbalState;

TYPE_GOLBAL_DATA::TYPE_GOLBAL_DATA()
{
    labelMode = 0;
    classNames = {
        "0:人",
        "1:轿车",
        "2:客车",
        "3:货车",
        "4:自行车",
        "5:摩托车",
        "6:道路实线",
        "7:道路虚线",
        "8:人行道",
        "9:路灯",
        "10:电线杆",
        "11:单后视镜",
        "12:双后视镜",
        "13:特征区域",
        "14:标尺",
        "15:基准线",
        "16:基准点",
        "17:null"
    };
    classColors = {
        QColor(128, 0, 0),
        QColor(0, 128, 0),
        QColor(128, 128, 0),
        QColor(0, 0, 128),
        QColor(128, 0, 128),
        QColor(0, 128, 128),
        QColor(128, 128, 128),
        QColor(64, 0, 0),
        QColor(192, 0, 0),
        QColor(64, 128, 0),
        QColor(192, 128, 0),
        QColor(64, 0, 128),
        QColor(192, 0, 128),
        QColor(64, 128, 128),
        QColor(192, 128, 128),
        QColor(0, 64, 0),
        QColor(128, 64, 0),
        QColor(0, 192, 0),
        QColor(128, 192, 0),
        QColor(0, 64, 128),
        QColor(128, 64, 12)
    };
    classIshow.resize(classColors.size());
    for(int i=0;i!=classColors.size();i++)
        classIshow[i] = true;

    lineWidth = 4;
    fontColor = Qt::green;
    alpha = 60;
    fontSize = 24;
    pointSize = 10;
    themeStr = "dark";
    langStr = "zh";
}

TYPE_GOLBAL_DATA::~TYPE_GOLBAL_DATA()
{

}




void TYPE_GOLBAL_DATA::readSettingIni() {
    QSettings settings(SETTING_FILE_NAME, QSettings::IniFormat);

    // 读取基本类型参数
    labelMode = settings.value("Label/labelMode", 0).toInt();
    curIndex = settings.value("Label/curIndex", 0).toInt();
    lineWidth = settings.value("Style/lineWidth", 2).toInt();
    alpha = settings.value("Style/alpha", 100).toInt();
    fontSize = settings.value("Style/fontSize", 12).toInt();
    pointSize = settings.value("Style/pointSize", 5).toInt();

    // 读取颜色参数
    fontColor = QColor(settings.value("Style/fontColor", "#ffffff").toString());

    // 清空现有数据
    classNames.clear();
    classColors.clear();
    classIshow.clear();

    // 读取类别数量和类别数据
    int classCount = settings.value("Classes/count", 0).toInt();
    for (int i = 0; i < classCount; ++i) {
        QString nameKey = QString("Classes/name_%1").arg(i);
        QString colorKey = QString("Classes/color_%1").arg(i);
        QString isShowKey = QString("Classes/isShow_%1").arg(i);

        QString name = settings.value(nameKey, "").toString();
        QColor color = QColor(settings.value(colorKey, "#000000").toString());
        bool isShow = settings.value(isShowKey).toBool();

        if (!name.isEmpty()) {
            classNames.append(name);
            classColors.append(color);
            classIshow.append(isShow);
        }
    }

    // 确保至少有一个默认类别
    if (classNames.isEmpty()) {
        classNames.append("Default");
        classColors.append(Qt::red);
        classIshow.append(true);
    }

    // 确保当前索引有效
    if (curIndex < 0 || curIndex >= classNames.size()) {
        curIndex = 0;
    }

    workPath = settings.value("normal/workPath").toString();
    editPath = settings.value("normal/editPath").toString();
    themeStr = settings.value("normal/themeStr").toString();
    langStr = settings.value("normal/langStr").toString();
}

void TYPE_GOLBAL_DATA::writeSettingIni() {
    QSettings settings(SETTING_FILE_NAME, QSettings::IniFormat);

    // 写入基本类型参数
    settings.setValue("Label/labelMode", labelMode);
    settings.setValue("Label/curIndex", curIndex);
    settings.setValue("Style/lineWidth", lineWidth);
    settings.setValue("Style/fontColor", fontColor.name());
    settings.setValue("Style/alpha", alpha);
    settings.setValue("Style/fontSize", fontSize);
    settings.setValue("Style/pointSize", pointSize);

    // 写入类别数量和类别数据
    settings.setValue("Classes/count", classNames.size());
    for (int i = 0; i < classNames.size(); ++i) {
        QString nameKey = QString("Classes/name_%1").arg(i);
        QString colorKey = QString("Classes/color_%1").arg(i);
        QString isShowKey = QString("Classes/isShow_%1").arg(i);

        settings.setValue(nameKey, classNames[i]);
        settings.setValue(colorKey, classColors[i].name());
        //qDebug()<<classIshow[i];
        settings.setValue(isShowKey, classIshow[i]);
    }

    settings.setValue("normal/workPath", workPath);
    settings.setValue("normal/editPath", editPath);

    settings.setValue("normal/themeStr",themeStr);
    settings.setValue("normal/langStr",langStr);
    // 确保写入磁盘
    settings.sync();
}



GlobalSettingsDialog::GlobalSettingsDialog(TYPE_GOLBAL_DATA &globalData, QWidget *parent)
    : QDialog(parent),
      m_globalData(globalData)
{
    setWindowTitle(tr("全局设置"));
    resize(500, 800);

    // 创建模型
    m_classModel = new QStandardItemModel(this);

    // 创建控件和布局
    createWidgets();
    createLayouts();
    createConnections();

    // 初始化UI数据
    updateUiFromData();
}

GlobalSettingsDialog::~GlobalSettingsDialog()
{
}

void GlobalSettingsDialog::createWidgets()
{
    // 标注设置组
    m_labelModeCombo = new QComboBox(this);
    m_labelModeCombo->addItem(tr("矩形标注"), 0);
    m_labelModeCombo->addItem(tr("多边形标注"), 1);

    // 样式设置组
    m_lineWidthSpin = new QSpinBox(this);
    m_lineWidthSpin->setRange(1, 10);

    m_fontSizeSpin = new QSpinBox(this);
    m_fontSizeSpin->setRange(8, 36);

    m_pointSizeSpin = new QSpinBox(this);
    m_pointSizeSpin->setRange(1, 20);

    m_alphaSlider = new QSlider(Qt::Horizontal, this);
    m_alphaSlider->setRange(0, 255);

    m_fontColorBtn = new QPushButton(this);
    m_fontColorBtn->setFixedSize(24, 24);

    // 类别管理组
    m_classListView = new QListView(this);
    m_classListView->setModel(m_classModel);
    m_classListView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    m_addClassBtn = new QPushButton(tr("添加"), this);
    m_removeClassBtn = new QPushButton(tr("删除"), this);
    m_colorBtn = new QPushButton(tr("颜色"), this);
    m_rowLabel = new QLabel(tr(""), this);
    m_removeClassBtn->setEnabled(false);
    m_colorBtn->setEnabled(false);

    // 按钮盒
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
}

void GlobalSettingsDialog::createLayouts()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 标注设置组
    QGroupBox *labelGroup = new QGroupBox(tr("标注设置"), this);
    QFormLayout *labelLayout = new QFormLayout(labelGroup);
    labelLayout->addRow(tr("标注模式:"), m_labelModeCombo);
    labelGroup->setLayout(labelLayout);

    // 样式设置组
    QGroupBox *styleGroup = new QGroupBox(tr("样式设置"), this);
    QFormLayout *styleLayout = new QFormLayout(styleGroup);
    styleLayout->addRow(tr("线宽:"), m_lineWidthSpin);
    styleLayout->addRow(tr("字体大小:"), m_fontSizeSpin);
    styleLayout->addRow(tr("点大小:"), m_pointSizeSpin);
    styleLayout->addRow(tr("透明度:"), m_alphaSlider);

    QHBoxLayout *fontColorLayout = new QHBoxLayout();
    fontColorLayout->addWidget(new QLabel(tr("线和字体颜色:")));
    fontColorLayout->addWidget(m_fontColorBtn);
    fontColorLayout->addStretch();
    styleLayout->addRow(fontColorLayout);
    styleGroup->setLayout(styleLayout);

    // 类别管理组
    QGroupBox *classGroup = new QGroupBox(tr("类别管理"), this);
    QVBoxLayout *classLayout = new QVBoxLayout(classGroup);

    QHBoxLayout *classButtonLayout = new QHBoxLayout();
    classButtonLayout->addWidget(m_addClassBtn);
    classButtonLayout->addWidget(m_removeClassBtn);
    classButtonLayout->addWidget(m_colorBtn);
    classButtonLayout->addWidget(m_rowLabel);
    classButtonLayout->addStretch();

    classLayout->addWidget(m_classListView);
    classLayout->addLayout(classButtonLayout);
    classGroup->setLayout(classLayout);

    // 分隔线
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    // 组合所有部件
    mainLayout->addWidget(labelGroup);
    mainLayout->addWidget(styleGroup);
    mainLayout->addWidget(classGroup);
    mainLayout->addWidget(line);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);
}

void GlobalSettingsDialog::createConnections()
{
    connect(m_addClassBtn, &QPushButton::clicked, this, &GlobalSettingsDialog::onAddClass);
    connect(m_removeClassBtn, &QPushButton::clicked, this, &GlobalSettingsDialog::onRemoveClass);
    connect(m_classListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &GlobalSettingsDialog::onClassSelectionChanged);
    connect(m_colorBtn, &QPushButton::clicked, this, &GlobalSettingsDialog::onColorButtonClicked);
    connect(m_fontColorBtn, &QPushButton::clicked, this, &GlobalSettingsDialog::onFontColorButtonClicked);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void GlobalSettingsDialog::updateUiFromData()
{
    // 基本参数
    m_labelModeCombo->setCurrentIndex(m_globalData.labelMode);
    m_lineWidthSpin->setValue(m_globalData.lineWidth);
    m_fontSizeSpin->setValue(m_globalData.fontSize);
    m_pointSizeSpin->setValue(m_globalData.pointSize);
    m_alphaSlider->setValue(m_globalData.alpha);

    // 字体颜色
    QPixmap fontColorPixmap(16, 16);
    fontColorPixmap.fill(m_globalData.fontColor);
    m_fontColorBtn->setIcon(QIcon(fontColorPixmap));

    // 类别列表
    m_classModel->clear();
    for (int i = 0; i < m_globalData.classNames.size(); ++i) {
        QStandardItem *item = new QStandardItem(m_globalData.classNames[i]);
        item->setCheckable(true);  // 自动设置 Qt::CheckStateRole
        item->setCheckState(m_globalData.classIshow[i]?Qt::Checked:Qt::Unchecked); // 初始状态：未选中
        item->setData(m_globalData.classColors[i], Qt::DecorationRole);
        item->setEditable(true);
        m_classModel->appendRow(item);
    }

    // 设置当前选中项
    if (m_globalData.curIndex >= 0 && m_globalData.curIndex < m_classModel->rowCount()) {
        m_classListView->setCurrentIndex(m_classModel->index(m_globalData.curIndex, 0));
    }
}

void GlobalSettingsDialog::updateDataFromUi()
{
    // 基本参数
    m_globalData.labelMode = m_labelModeCombo->currentIndex();
    m_globalData.lineWidth = m_lineWidthSpin->value();
    m_globalData.fontSize = m_fontSizeSpin->value();
    m_globalData.pointSize = m_pointSizeSpin->value();
    m_globalData.alpha = m_alphaSlider->value();

    // 更新类别数据
    m_globalData.classNames.clear();
    m_globalData.classColors.clear();
    m_globalData.classIshow.clear();

    for (int i = 0; i < m_classModel->rowCount(); ++i) {
        QStandardItem *item = m_classModel->item(i);
        m_globalData.classNames.append(item->text());
        m_globalData.classColors.append(item->data(Qt::DecorationRole).value<QColor>());
        m_globalData.classIshow.append(item->checkState());
    }

    // 更新当前选中索引
    QModelIndexList selected = m_classListView->selectionModel()->selectedIndexes();
    if (!selected.isEmpty()) {
        m_globalData.curIndex = selected.first().row();
    } else {
        m_globalData.curIndex = 0;
    }
}

void GlobalSettingsDialog::onAddClass()
{
    QString str = QString::number(m_classModel->rowCount()) + tr(":新类别");
    bool ok;
    QString className = QInputDialog::getText(this, tr("添加类别"),
                                            tr("请输入类别名称:"),
                                            QLineEdit::Normal,
                                            str,
                                            &ok);

    if (ok && !className.isEmpty()) {
        QStandardItem *item = new QStandardItem(className);
        // 启用复选框
        item->setCheckable(true);  // 自动设置 Qt::CheckStateRole
        item->setCheckState(Qt::Checked); // 初始状态：未选中
        item->setData(QColor(Qt::red), Qt::DecorationRole); // 默认红色
        item->setEditable(true);
        m_classModel->appendRow(item);

        // 选中新添加的项
        m_classListView->setCurrentIndex(m_classModel->index(m_classModel->rowCount() - 1, 0));
    }
}

void GlobalSettingsDialog::onRemoveClass()
{
    QModelIndexList selected = m_classListView->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个类别"));
        return;
    }

    if (m_classModel->rowCount() <= 1) {
        QMessageBox::warning(this, tr("警告"), tr("至少需要保留一个类别"));
        return;
    }

    int row = selected.first().row();
    m_classModel->removeRow(row);

    // 自动选择下一个项
    if (row >= m_classModel->rowCount()) {
        row = m_classModel->rowCount() - 1;
    }
    if (row >= 0) {
        m_classListView->setCurrentIndex(m_classModel->index(row, 0));
    }
}

void GlobalSettingsDialog::onClassSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    if (selected.isEmpty()) {
        m_removeClassBtn->setEnabled(false);
        m_colorBtn->setEnabled(false);
        return;
    }

    m_removeClassBtn->setEnabled(true);
    m_colorBtn->setEnabled(true);

    QModelIndex index = selected.indexes().first();
    QStandardItem *item = m_classModel->itemFromIndex(index);
    m_currentColor = item->data(Qt::DecorationRole).value<QColor>();

    // 更新颜色按钮图标
    QPixmap pixmap(16, 16);
    pixmap.fill(m_currentColor);
    m_colorBtn->setIcon(QIcon(pixmap));

    m_rowLabel->setText(QString::number(index.row()));
}

void GlobalSettingsDialog::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_currentColor, this, tr("选择颜色"));
    if (color.isValid()) {
        m_currentColor = color;

        // 更新当前选中项的颜色
        QModelIndexList selected = m_classListView->selectionModel()->selectedIndexes();
        if (!selected.isEmpty()) {
            QStandardItem *item = m_classModel->itemFromIndex(selected.first());
            item->setData(m_currentColor, Qt::DecorationRole);

            // 更新颜色按钮图标
            QPixmap pixmap(16, 16);
            pixmap.fill(m_currentColor);
            m_colorBtn->setIcon(QIcon(pixmap));
        }
    }
}

void GlobalSettingsDialog::onFontColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_globalData.fontColor, this, tr("选择字体颜色"));
    if (color.isValid()) {
        m_globalData.fontColor = color;

        // 更新颜色按钮图标
        QPixmap pixmap(16, 16);
        pixmap.fill(color);
        m_fontColorBtn->setIcon(QIcon(pixmap));
    }
}

void GlobalSettingsDialog::accept()
{
    // 验证数据
    if (m_classModel->rowCount() == 0) {
        QMessageBox::warning(this, tr("错误"), tr("至少需要一个类别"));
        return;
    }

    // 更新数据
    updateDataFromUi();

    // 保存到文件
    m_globalData.writeSettingIni();

    QDialog::accept();
}
