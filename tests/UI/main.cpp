#include <QApplication>

int main(int _argc, char* _argv[])
{
    QApplication app{ _argc, _argv };
    app.setApplicationName("Blokus AI Tests");
    app.setApplicationVersion("1.0.0");

    return app.exec();
}
