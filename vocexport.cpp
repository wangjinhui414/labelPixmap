#include "vocexport.h"
#include <QImageReader>
#include <QDebug>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPainter>
#include <QApplication>
#include <QPainterPath>

VOCExport::VOCExport(QObject *parent) : QObject(parent) {}

void VOCExport::setExportParam(VOCExport::ExportMode mode,const QString &inputDir,
                             const QString &outputDir, const QVector<QString> &classNames,
                            int trainSplit , int valSplit , int testSplit ) {
    m_inputDir = inputDir;
    m_outputDir = outputDir;
    m_trainSplit = trainSplit;
    m_valSplit = valSplit;
    m_testSplit = testSplit;
    m_mode = mode;
    m_classNames = classNames;

}
void VOCExport::creatOutputDir()
{
    QDir diro(m_outputDir);
    if (!diro.exists()) {
        diro.mkpath(".");
    }
    else{
        if(!diro.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty())//路径不为空提示用户是否删除
        {
            // 创建消息框询问用户
             QMessageBox::StandardButton reply;
             reply = QMessageBox::question(nullptr,
                                          "警告",
                                          "目录为非空,YES删除再输出。NO覆盖输出。",
                                          QMessageBox::Yes | QMessageBox::No);

             if (reply == QMessageBox::Yes) {
                    diro.removeRecursively();
             }
        }

    }
}
void VOCExport::exportDataset()
{
    creatOutputDir();
    QDir dir(m_inputDir);
    if (!dir.exists()) {
        qWarning() << "Input directory does not exist:" << m_inputDir;
        return;
    }

    // Create VOC output directories
    QDir(m_outputDir).mkpath("Annotations");
    QDir(m_outputDir).mkpath("JPEGImages");
    QDir(m_outputDir).mkpath("SegmentationClass");
    QDir(m_outputDir).mkpath("ImageSets");

    // Process all .path files
    QStringList filters;
    filters << "*.path";
    QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files);
    // 随机打乱文件列表
    shuffleFileList(fileInfoList);

    // 根据比例划分训练集、验证集和测试集
    int totalFiles = fileInfoList.size();
    int sum = m_trainSplit + m_valSplit + m_testSplit;
    int trainCount = static_cast<int>(totalFiles * m_trainSplit*1.0/sum);
    int valCount = static_cast<int>(totalFiles * m_valSplit*1.0/sum);

    QProgressDialog progressDialog;
    progressDialog.setWindowTitle(tr("处理进度"));
    progressDialog.setLabelText(tr("正在转换..."));
    progressDialog.setRange(0, totalFiles);
    progressDialog.setModal(true); // 设置为模态对话框，阻塞用户其他操作
    progressDialog.show();

    QStringList trainList;
    QStringList valList;
    QStringList testList;
    QStringList trainValList;
    for (int i = 0; i < totalFiles; ++i) {
        if (i < trainCount) {
            trainList.append(fileInfoList[i].baseName());
            trainValList.append(fileInfoList[i].baseName());
        } else if (i < trainCount + valCount) {
            valList.append(fileInfoList[i].baseName());
            trainValList.append(fileInfoList[i].baseName());
        } else {
            testList.append(fileInfoList[i].baseName());
        }
        processFile(fileInfoList[i].absoluteFilePath(),  m_classNames);

        progressDialog.setValue(i);
        QApplication::processEvents();
    }
    progressDialog.setValue(totalFiles);
    // 创建ImageSets及其子目录
    QString imageSetsPath = QDir(m_outputDir).filePath("ImageSets");
     QStringList subdirs = {"Main", "Segmentation"};//, "Layout", "Action"
     foreach (const QString &subdir, subdirs) {
         QString path = QString("%1/%2").arg(imageSetsPath).arg(subdir);
         QDir().mkpath(path);
     }
     QString txtPath ;
     if(m_mode == VOCExport::OBJECT_DETECTION)
         txtPath = imageSetsPath + "/Main";
     else if(m_mode == VOCExport::INSTANCE_SEGMENTATION)
         txtPath = imageSetsPath + "/Segmentation";
    generalTxtFile(txtPath,"/train.txt",trainList);
    generalTxtFile(txtPath,"/trainval.txt",trainValList);
    generalTxtFile(txtPath,"/test.txt",testList);
    generalTxtFile(txtPath,"/val.txt",valList);

}

void VOCExport::shuffleFileList(QFileInfoList &fileNames)
{
   // 随机打乱文件列表
   for (int i = fileNames.size() - 1; i > 0; --i) {
       int j = QRandomGenerator::global()->bounded(i + 1);
       fileNames.swapItemsAt(i, j);
   }
}

void VOCExport::processFile(const QString &filePath, const QVector<QString> &classNames) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath;
        return;
    }

    QTextStream in(&file);
    QString imageName = QFileInfo(filePath).baseName() + ".jpg";
    QString imagePath = m_outputDir + "/JPEGImages/" + imageName;

    // Copy image (assuming it exists in same directory as .path file)
    QFile::copy(QFileInfo(filePath).path() + "/" + imageName, imagePath);

    // Get image size
    QImageReader reader(imagePath);
    if (!reader.canRead()) {
        qWarning() << "Cannot read image:" << imagePath;
        return;
    }
    QSize imageSize = reader.size();

    // Parse annotations
    QList<QPair<int, QPolygonF>> objects1;
    QList<QPair<int, QPolygonF>> objects2;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 3) {
            qWarning() << "Invalid line in" << filePath << ":" << line;
            continue;
        }

        QString shapeType = parts[0];
        int classId = parts[1].toInt();
        if (classId < 0 || classId >= classNames.size()) {
            qWarning() << "Invalid class ID in" << filePath << ":" << classId;
            continue;
        }

        QPolygonF polygon;
        for (int i = 2; i < parts.size(); i += 2) {
            if (i + 1 >= parts.size()) break;
            qreal x = parts[i].toDouble();
            qreal y = parts[i+1].toDouble();
            polygon << QPointF(x, y);
        }

        // For rectangles, ensure we have exactly 4 points
        if (shapeType == "rect" && polygon.size() == 4) {
            objects1.append(qMakePair(classId, polygon));
        } else if (shapeType == "path" && polygon.size() >= 3) {

            objects2.append(qMakePair(classId, polygon));
        } else {
            qWarning() << "Invalid shape data in" << filePath << ":" << line;
        }
    }

    // Write VOC annotation
    if(m_mode == VOCExport::OBJECT_DETECTION )
    {
        QString annotationPath = m_outputDir + "/Annotations/" + QFileInfo(filePath).baseName() + ".xml";
        writeVOCAnnotation(imageName, imageSize, objects1, classNames, annotationPath);
    }
    else if(m_mode == VOCExport::INSTANCE_SEGMENTATION )
    {
        QString annotationPath = m_outputDir + "/SegmentationClass/" + QFileInfo(filePath).baseName() + ".png";
        writeVOCSegmentationClass(imageName, imageSize, objects2, classNames, annotationPath);
    }

}

void VOCExport::writeVOCAnnotation(const QString &imageName, const QSize &imageSize,
                                  const QList<QPair<int, QPolygonF>> &objects,
                                  const QVector<QString> &classNames,
                                  const QString &outputPath) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create annotation file:" << outputPath;
        return;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    xml.writeStartElement("annotation");
    xml.writeTextElement("folder", "VOC2012");
    xml.writeTextElement("filename", imageName);

    // Image size
    xml.writeStartElement("size");
    xml.writeTextElement("width", QString::number(imageSize.width()));
    xml.writeTextElement("height", QString::number(imageSize.height()));
    xml.writeTextElement("depth", "3"); // Assuming RGB
    xml.writeEndElement(); // size

    xml.writeTextElement("segmented", "0");

    // Objects
    foreach (const auto &obj, objects) {
        int classId = obj.first;
        const QPolygonF &polygon = obj.second;

        xml.writeStartElement("object");
        xml.writeTextElement("name", classNames[classId]);
        xml.writeTextElement("pose", "Unspecified");
        xml.writeTextElement("truncated", "0");
        xml.writeTextElement("difficult", "0");

        // Bounding box (calculate from polygon)
        QRectF bbox = polygon.boundingRect();
        xml.writeStartElement("bndbox");
        xml.writeTextElement("xmin", QString::number(bbox.left()*imageSize.width()));
        xml.writeTextElement("ymin", QString::number(bbox.top()*imageSize.height()));
        xml.writeTextElement("xmax", QString::number(bbox.right()*imageSize.width()));
        xml.writeTextElement("ymax", QString::number(bbox.bottom()*imageSize.height()));
        xml.writeEndElement(); // bndbox

        xml.writeEndElement(); // object
    }

    xml.writeEndElement(); // annotation
    xml.writeEndDocument();
}

void VOCExport::writeVOCSegmentationClass(
    const QString &imageName,
    const QSize &imageSize,
    const QList<QPair<int, QPolygonF>> &objects,
    const QVector<QString> &classNames,
    const QString &outputPath)
{
    Q_UNUSED(imageName);
    Q_UNUSED(classNames);

    // 1. 创建8位灰度图像（初始填充为0=背景）
    QImage mask(imageSize, QImage::Format_Indexed8);
    mask.fill(0);

    // 2. 设置VOC标准调色板（21类）
    QVector<QRgb> colorTable = {
//        qRgb(0, 0, 0),        // 0: background
//        qRgb(128, 0, 0),      // 1: aeroplane
//        qRgb(0, 128, 0),      // 2: bicycle
//        qRgb(128, 128, 0),    // 3: bird
//        qRgb(0, 0, 128),      // 4: boat
//        qRgb(128, 0, 128),    // 5: bottle
//        qRgb(0, 128, 128),    // 6: bus
//        qRgb(128, 128, 128),  // 7: car
//        qRgb(64, 0, 0),       // 8: cat
//        qRgb(192, 0, 0),      // 9: chair
//        qRgb(64, 128, 0),     // 10: cow
//        qRgb(192, 128, 0),    // 11: diningtable
//        qRgb(64, 0, 128),     // 12: dog
//        qRgb(192, 0, 128),    // 13: horse
//        qRgb(64, 128, 128),   // 14: motorbike
//        qRgb(192, 128, 128),  // 15: person
//        qRgb(0, 64, 0),       // 16: pottedplant
//        qRgb(128, 64, 0),     // 17: sheep
//        qRgb(0, 192, 0),      // 18: sofa
//        qRgb(128, 192, 0),    // 19: train
//        qRgb(0, 64, 128)      // 20: tvmonitor
    };
    colorTable.clear();
    colorTable.append(qRgb(0, 0, 0));
    for(auto one:golbalState.classColors){
        colorTable.append(one.rgb());
    }
    mask.setColorTable(colorTable);
    // 3. 处理归一化坐标并填充多边形
    for (const auto &obj : objects) {
        int classId = obj.first+1;
        QPolygonF normalizedPoly = obj.second;

        // 转换为像素坐标的多边形
        QPolygon pixelPoly;
        for (const QPointF& p : normalizedPoly) {
            int x = static_cast<int>(p.x() * imageSize.width());
            int y = static_cast<int>(p.y() * imageSize.height());
            pixelPoly << QPoint(x, y);
        }

        // 创建QPainterPath用于精确填充
        QPainterPath path;
        path.addPolygon(pixelPoly);

        // 获取多边形的边界矩形
        QRect bounds = pixelPoly.boundingRect().intersected(mask.rect());

        // 遍历边界矩形内的每个像素
        for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
            for (int x = bounds.left(); x <= bounds.right(); ++x) {
                if (path.contains(QPointF(x + 0.5, y + 0.5))) { // 检查像素中心点
                    mask.setPixel(x, y, classId);
                }
            }
        }
    }

    // 4. 保存为PNG
    if (!mask.save(outputPath, "PNG")) {
        qWarning() << "Failed to save segmentation mask:" << outputPath;
    } else {
    }
}

void VOCExport::generalTxtFile(QString pathName, QString txtName,QStringList strlst)
{
    QFile trainFile(pathName + txtName);
    if (trainFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&trainFile);
        foreach (const QString &lineStr, strlst) {
            out << lineStr << "\n";
        }
        trainFile.close();

    }
}
