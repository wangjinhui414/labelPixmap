#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H
#include <QDialog>
#include <QStandardItemModel>
#include <QListView>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QObject>
#include <QColor>
#include <QLabel>

#define LABEL_RECT_MODE     0
#define LABEL_POLYGON_MODE  1

class TYPE_GOLBAL_DATA
{
public:
    TYPE_GOLBAL_DATA();
    ~TYPE_GOLBAL_DATA();
    QString curCsName(){return classNames[curIndex];}
    QColor curCsColor(){return classColors[curIndex];}
    int curCsIndex(){return curIndex;}

    //需要完成的函数
    void readSettingIni();
    void writeSettingIni();
    //需要读写的参数
    int labelMode;//标注模式
    QVector<QString> classNames;//类别名字
    QVector<QColor> classColors;//类别颜色
    int curIndex;//当前类别
    int lineWidth;//线宽
    QColor fontColor;//字体颜色
    int alpha;//蒙版透明度
    int fontSize;//字体大小
    int pointSize;//远点大小
    //工作目录
    QString workPath;
    QString editPath;

    QString themeStr;
    QString langStr;
};





class GlobalSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalSettingsDialog(TYPE_GOLBAL_DATA &globalData, QWidget *parent = nullptr);
    ~GlobalSettingsDialog();

private slots:
    void onAddClass();
    void onRemoveClass();
    void onClassSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onColorButtonClicked();
    void onFontColorButtonClicked();
    void accept() override;

private:
    TYPE_GOLBAL_DATA &m_globalData;
    QStandardItemModel *m_classModel;
    QColor m_currentColor;

    // 控件成员
    QComboBox *m_labelModeCombo;
    QSpinBox *m_lineWidthSpin;
    QSpinBox *m_fontSizeSpin;
    QSpinBox *m_pointSizeSpin;
    QSlider *m_alphaSlider;
    QPushButton *m_fontColorBtn;
    QListView *m_classListView;
    QPushButton *m_addClassBtn;
    QPushButton *m_removeClassBtn;
    QPushButton *m_colorBtn;
    QLabel *m_rowLabel;
    QDialogButtonBox *m_buttonBox;

    void createWidgets();
    void createLayouts();
    void createConnections();
    void updateUiFromData();
    void updateDataFromUi();
};



extern TYPE_GOLBAL_DATA golbalState;



#endif // GLOBAL_DATA_H
