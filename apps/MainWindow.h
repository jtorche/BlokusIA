#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* _parent = nullptr);
    ~MainWindow();

private slots:
    void updateLanguage();
    void about();

private:
    void setupActions();
    void setupConnections();

private:
    Ui::MainWindow* m_ui;
};
