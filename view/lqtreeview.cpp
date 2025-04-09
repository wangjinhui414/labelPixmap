#include "lqtreeview.h"


LQTreeView::LQTreeView(QWidget *parent):QTreeView(parent)
{
    createWidgets();
}

LQTreeView::~LQTreeView()
{

}

void LQTreeView::createWidgets()
{
    // 创建文件系统模型
    m_fileModel = new CustomFileSystemModel();
    m_fileModel->setRootPath(QDir::rootPath());
    m_fileModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot); // 过滤掉 "." 和 ".."
    //m_fileModel->setNameFilters({"*.jpg", "*.png", "*.bmp", "*.jpeg",".jfif",".path"}); // 设置图片文件过滤器
   // m_fileModel->setNameFilterDisables(false); // 隐藏不匹配的文件
    //设置初始参数
    this->setModel(m_fileModel);

    this->setColumnHidden(1, true); // 隐藏大小列
    this->setColumnHidden(2, true); // 隐藏类型列
    this->setColumnHidden(3, true); // 隐藏修改日期列


}


