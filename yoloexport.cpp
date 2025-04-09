#include "yoloexport.h"
#include <iostream>
#include <QRandomGenerator>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QPolygonF>
#include <QProgressDialog>
#include <QObject>
#include <QApplication>
#include <QMessageBox>

void YOLOExporter::setExportParam(ExportMode mode, const QString& inputDir, const QString& outputDir,
                                 const QVector<QString> classNames, int trainSplit, int valSplit, int testSplit)
{
    m_mode=mode;
    m_inputDir=inputDir;
    m_outputDir=outputDir;
    m_trainSplit=trainSplit;
    m_valSplit=valSplit;
    m_testSplit=testSplit;
    m_classNames=classNames;
}


void YOLOExporter::exportDataset() {
    // 创建必要的目录结构
    createOutputDirs();

    QDir inputDir(m_inputDir);
    QStringList fileNames = inputDir.entryList(QStringList() << "*.path", QDir::Files);

    // 随机打乱文件列表
    shuffleFileList(fileNames);

    // 根据比例划分训练集、验证集和测试集
    int totalFiles = fileNames.size();
    int sum = m_trainSplit + m_valSplit + m_testSplit;
    int trainCount = static_cast<int>(totalFiles * m_trainSplit*1.0/sum);
    int valCount = static_cast<int>(totalFiles * m_valSplit*1.0/sum);
    //int testCount = totalFiles - trainCount - valCount;


    // 创建进度条对话框
    QProgressDialog progressDialog;
    progressDialog.setWindowTitle(tr("处理进度"));
    progressDialog.setLabelText(tr("正在转换..."));
    progressDialog.setRange(0, totalFiles);
    progressDialog.setModal(true); // 设置为模态对话框，阻塞用户其他操作
    progressDialog.show();
    //qDebug()<<trainCount;
    for (int i = 0; i < totalFiles; ++i) {
        QString fileName = fileNames[i];
        QString destinationDir;
        if (i < trainCount) {
            destinationDir = "/train/";
        } else if (i < trainCount + valCount) {
            destinationDir = "/val/";
        } else {
            destinationDir = "/test/";
        }
        processFile(fileName, destinationDir);
        progressDialog.setValue(i);
        QApplication::processEvents();
    }
    progressDialog.setValue(totalFiles);
    // 创建数据集配置文件
    createYmalFile();
}

void YOLOExporter::createOutputDirs() {

    QDir dir(m_outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    else{
        if(!dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty())//路径不为空提示用户是否删除
        {
            // 创建消息框询问用户
             QMessageBox::StandardButton reply;
             reply = QMessageBox::question(nullptr,
                                          "警告",
                                          "目录为非空,YES删除再输出。NO覆盖输出。",
                                          QMessageBox::Yes | QMessageBox::No);

             if (reply == QMessageBox::Yes) {
                    dir.removeRecursively();
             }
        }

    }

    QDir imagesDir(m_outputDir + "/images");
    if (!imagesDir.exists()) {
        imagesDir.mkpath(".");
    }

    QDir labelsDir(m_outputDir + "/labels");
    if (!labelsDir.exists()) {
        labelsDir.mkpath(".");
    }

    imagesDir.mkpath("train");
    imagesDir.mkpath("val");
    imagesDir.mkpath("test");

    labelsDir.mkpath("train");
    labelsDir.mkpath("val");
    labelsDir.mkpath("test");
}

void YOLOExporter::shuffleFileList(QStringList& fileNames) {
    // 随机打乱文件列表
    for (int i = fileNames.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        fileNames.swapItemsAt(i, j);
    }
}

void YOLOExporter::processFile(const QString& fileName, const QString& destinationDir) {


    //判断原始图片文件
    QString piaxName = "";
    QStringList pixExtNames{".jpg",".png",".jpeg",".jfif"};
    for(auto one:pixExtNames){
        QString fileBaseName = fileName;
        piaxName = m_inputDir + "/" + fileBaseName.replace(".path", one);
        if(QFile::exists(piaxName)){
            break;
        }
        piaxName = "";
    }
    if(piaxName.isEmpty()) return;
    QString outPixname = m_outputDir + "/images" + destinationDir + QFileInfo(piaxName).fileName();
    QFile::copy(piaxName,outPixname);
    //打开path文件
    QString pathFileName = m_inputDir + "/" + fileName;
    QFile inputFile(pathFileName);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Failed to open file: " << fileName.toStdString() << std::endl;
        return;
    }
    //创建输出txt文件
    QString labelName = fileName;
    labelName.replace(".path", ".txt");
    QString outputFileName = m_outputDir + "/labels" + destinationDir + labelName;
    QFile outputFile(outputFileName);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to open output file: " << outputFileName.toStdString() << std::endl;
        return;
    }
    //qDebug()<<piaxName<<"  "<<outPixname;
    //qDebug()<<pathFileName<<"  "<<outputFileName;


    //YOLO txt文件转换
    QTextStream in(&inputFile);
    QTextStream out(&outputFile);

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList tokens = line.split(" ");
        if(tokens.size()<4) continue;
        //qDebug()<<tokens;
        QString type = tokens[0];
        int classId = tokens[1].toInt();
        QVector<QPointF> points;
        for (int i = 2; i < tokens.size(); i += 2) {
            if((i+1)>=tokens.size()) continue;
            qreal x = tokens[i].toDouble();
            qreal y = tokens[i + 1].toDouble();
            points.append(QPointF(x, y));
        }

        // 根据标注类型导出
        if (type == "rect") {
            handleRect(classId, points, out);
        } else if (type == "path") {
            handlePath(classId, points, out);
        }
    }
    //qDebug()<<"**************************";
    inputFile.close();
    outputFile.close();
}

void YOLOExporter::handleRect(int classId, const QVector<QPointF>& points, QTextStream& out) {
    if (m_mode == OBJECT_DETECTION) {
        // 导出为目标检测的矩形框
        QPolygonF polygon;
        polygon.append(points);
        QRectF rect = polygon.boundingRect();
        out << classId << " "
            << (rect.left() + rect.right()) / 2 << " "
            << (rect.top() + rect.bottom()) / 2 << " "
            << rect.width() << " "
            << rect.height() << "\n";
    }
    else if (m_mode == ROTATED_RECTANGLE) {
        // 导出旋转矩形框
        out << classId ;
        for(auto one:points)
        {
            out<<" "<<one.x()<<" "<<one.y();
        }
        out<<"\n";
    }
}

void YOLOExporter::handlePath(int classId, const QVector<QPointF>& points, QTextStream& out) {
    if (m_mode == INSTANCE_SEGMENTATION) {
        // 导出为实例分割
        out << classId ;
        for (const QPointF& point : points) {
            out<< " " << point.x() << " " << point.y();
        }
        out << "\n";
    }
}

void YOLOExporter::createYmalFile() {
    QFile dataFile(m_outputDir + "/classes.yaml");
    if (!dataFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to open classes.yaml" << std::endl;
        return;
    }

    QTextStream out(&dataFile);
    out << "path: " << m_outputDir << "\n";
    out << "train: images/train\n";
    out << "val: images/val\n";
    out << "test: images/test\n\n\n";
    out << "names:\n";
    for(int i=0;i!=m_classNames.size();i++)
    {
        out << "  "<<QString::number(i)<<": "<<m_classNames[i]<<"\n";
    }
    dataFile.close();
}


