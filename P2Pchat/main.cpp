#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString addr = argc > 2 ? QString(argv[1]) : "";
    quint16 port = argc > 2 ? QString(argv[2]).toUShort() : 0;
    MainWindow w = MainWindow(nullptr, addr, port);
    w.show();
    return a.exec();
}
