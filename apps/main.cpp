#include <QApplication>

#include "MainWindow.h"
#include "themes/ThemeManager.h"

int main(int _argc, char* _argv[])
{
    QApplication app{ _argc, _argv };
    app.setApplicationName("Blokus");
    app.setApplicationVersion("1.0.0");

    blokusUi::ThemeManager::getInstance();

    blokusUi::MainWindow window;
    window.show();
    return app.exec();
}
