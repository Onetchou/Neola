#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    qDebug() << "Start of the program";
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
