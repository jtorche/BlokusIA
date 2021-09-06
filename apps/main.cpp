#include "MainWindow.h"

#include <QApplication>

int main(int _argc, char* _argv[])
{
    QApplication app(_argc, _argv);
    MainWindow window;
    window.show();
    return app.exec();
}