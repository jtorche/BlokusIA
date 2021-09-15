#include <QApplication>

#include "i18n/TranslationManager.h"
#include "themes/ThemeManager.h"

#include "MainWindow.h"

int main(int _argc, char* _argv[])
{
    QApplication app{ _argc, _argv };
    app.setApplicationName("Blokus");
    app.setApplicationVersion("1.0.0");

    TRANSLATION_MANAGER;
    THEME_MANAGER;

    blokusUi::MainWindow window;
    window.show();
    return app.exec();
}
