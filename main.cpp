#include "mainwindow.h"
#include "segmentation_techniques.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //Segmentation_techniques seg_tech;
    //seg_tech.Local_Otsu;
    w.show();
    
    return a.exec();
}
