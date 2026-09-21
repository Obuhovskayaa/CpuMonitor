#include <QApplication>
#include "mainwindow.h"
#include "AppCore.h" 

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    qRegisterMetaType<CpuPacket>("CpuPacket");
    AppCore core;
    MainWindow w;
    w.initConnections(&core);
    w.show();
    return a.exec();
}