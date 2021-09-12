#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QActionGroup>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* _parent)
    : QMainWindow(_parent)
    , m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    setupActions();
    setupConnections();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::updateLanguage()
{
}

void MainWindow::setupActions()
{
    auto languagesGroup = new QActionGroup(this);
    languagesGroup->addAction(m_ui->m_actionSystemLocale);
    languagesGroup->addAction(m_ui->m_actionEnglish);
    languagesGroup->addAction(m_ui->m_actionFrench);
}

void MainWindow::setupConnections()
{
    // Game
    connect(m_ui->m_actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    // Language
    connect(m_ui->m_actionSystemLocale, SIGNAL(triggered()), this, SLOT(updateLanguage()));
    connect(m_ui->m_actionEnglish, SIGNAL(triggered()), this, SLOT(updateLanguage()));
    connect(m_ui->m_actionFrench, SIGNAL(triggered()), this, SLOT(updateLanguage()));

    // About
    connect(m_ui->m_actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(m_ui->m_actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::about()
{
    QMessageBox::about(
        this,
        tr("About"),
        "<h1>" + QApplication::applicationName() + "</h1>"
        + "<h2>" + tr("Version: ") + QApplication::applicationVersion() + "</h2>"
        + tr("A graphical application for the Blokus game."));
}
