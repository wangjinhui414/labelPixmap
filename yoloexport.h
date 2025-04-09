#ifndef YOLOEXPORT_H
#define YOLOEXPORT_H

#include <QObject>
#include <QtCore>
#include <QVector>
#include <QPointF>
#include <QString>
#include <QFile>

class YOLOExporter : public QObject
{
public:
    enum ExportMode {
        OBJECT_DETECTION=0,
        ROTATED_RECTANGLE,
        INSTANCE_SEGMENTATION
    };
    YOLOExporter(QObject *parent=nullptr):QObject(parent){}
    ~YOLOExporter(){}
    void setExportParam(ExportMode mode, const QString& inputDir, const QString& outputDir,
                    const QVector<QString> classNames, int trainSplit = 5, int valSplit = 1, int testSplit = 1);
    void exportDataset();

private:
    ExportMode m_mode;
    QString m_inputDir;
    QString m_outputDir;
    QVector<QString> m_classNames;
    int m_trainSplit;
    int m_valSplit;
    int m_testSplit;

    void createOutputDirs();
    void shuffleFileList(QStringList& fileNames);
    void processFile(const QString& fileName, const QString& destinationDir);
    void handleRect(int classId, const QVector<QPointF>& points, QTextStream& out);
    void handlePath(int classId, const QVector<QPointF>& points, QTextStream& out);
    void createYmalFile();
};




#endif // YOLOEXPORT_H
