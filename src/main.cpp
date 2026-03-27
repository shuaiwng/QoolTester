#include "mainwindow.h"


int main(int argc, char *argv[]){

    QApplication app(argc, argv);
    MainWindow w_main;

    app.setStyle("Fusion");
    app.setQuitOnLastWindowClosed(false);

    return app.exec();
}

