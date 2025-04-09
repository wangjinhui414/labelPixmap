#ifndef CONVERTER_H
#define CONVERTER_H


#include <QObject>
#include <QDir>
#include <QFile>
#include <QXmlStreamWriter>
#include "global_data.h"

class VOCExport : public QObject
{
    Q_OBJECT
public:
    enum ExportMode {
        OBJECT_DETECTION=0,
        INSTANCE_SEGMENTATION
    };
    explicit VOCExport(QObject *parent = nullptr);
    void setExportParam(VOCExport::ExportMode mode,const QString &inputDir, const QString &outputDir,
                      const QVector<QString> &classNames,
                       int trainSplit = 5, int valSplit = 1, int testSplit = 1);
//    void setExportParam(ExportMode mode, const QString& inputDir, const QString& outputDir,
//                    const QVector<QString> classNames,
//                        int trainSplit = 5, int valSplit = 1, int testSplit = 1);
    void exportDataset();

    void shuffleFileList(QFileInfoList& fileNames);
private:
    void processFile(const QString &filePath,  const QVector<QString> &classNames);
    void writeVOCAnnotation(const QString &imageName, const QSize &imageSize,
                           const QList<QPair<int, QPolygonF>> &objects,
                           const QVector<QString> &classNames,
                           const QString &outputPath);
    void writeVOCSegmentationClass(const QString &imageName, const QSize &imageSize,
                           const QList<QPair<int, QPolygonF>> &objects,
                           const QVector<QString> &classNames,
                           const QString &outputPath);
    void generalTxtFile(QString pathName,QString txtName,QStringList strlst);
    void creatOutputDir();
    QString m_inputDir;
    QString m_outputDir;
    ExportMode m_mode;
    int m_trainSplit=0;
    int m_valSplit=0;
    int m_testSplit=0;
    QVector<QString> m_classNames;
};



#endif // CONVERTER_H
