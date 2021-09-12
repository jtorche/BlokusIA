#include <QApplication>

#include "MainWindow.h"

int main(int _argc, char* _argv[])
{
    QApplication app{ _argc, _argv };
    app.setApplicationName("Blokus");
    app.setApplicationVersion("1.0.0");

    MainWindow window;
    window.show();
    return app.exec();
}
