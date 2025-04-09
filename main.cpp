#include "mainwindow.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QTranslator translator;
//    if (translator.load(":/translations/LabelPixmap_en.qm")) {
//        qApp->installTranslator(&translator);
//    }

    MainWindow w;
    w.show();
    return a.exec();
}
