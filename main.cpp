#include <iostream>
#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
    srand(time(0));
    QApplication app(argc, argv);
    MainWindow window(&app);
    window.show();

    return app.exec();
}
