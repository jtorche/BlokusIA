#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QActionGroup>
#include <QMessageBox>

#include "themes/ThemeManager.h"

namespace blokusUi
{
    MainWindow::MainWindow(QWidget* _parent)
        : QMainWindow(_parent)
        , m_ui(new Ui::MainWindow)
    {
        setupTranslator(QLocale{});

        m_ui->setupUi(this);

        setupActions();
        setupConnections();
    }

    MainWindow::~MainWindow()
    {
        delete m_ui;
    }

    void MainWindow::changeEvent(QEvent* _event)
    {
        if (_event->type() == QEvent::LanguageChange)
        {
            m_ui->retranslateUi(this);
        }
    }

    void MainWindow::setupActions()
    {
        // Language
        auto languagesGroup = new QActionGroup{ this };
        languagesGroup->addAction(m_ui->m_actionSystemLocale);
        languagesGroup->addAction(m_ui->m_actionEnglish);
        languagesGroup->addAction(m_ui->m_actionFrench);

        // Theme
        auto themesGroup = new QActionGroup{ this };
        themesGroup->addAction(m_ui->m_actionDarkTheme);
        themesGroup->addAction(m_ui->m_actionLightTheme);
        QAction* themeToUse = THEME_MANAGER.isDarkTheme()
            ? m_ui->m_actionDarkTheme
            : m_ui->m_actionLightTheme;
        themeToUse->setChecked(true);
    }

    void MainWindow::setupConnections()
    {
        // Game
        connect(m_ui->m_actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

        // Language
        connect(m_ui->m_actionSystemLocale, SIGNAL(triggered()), this, SLOT(updateLanguage()));
        connect(m_ui->m_actionEnglish, SIGNAL(triggered()), this, SLOT(updateLanguage()));
        connect(m_ui->m_actionFrench, SIGNAL(triggered()), this, SLOT(updateLanguage()));

        // Theme
        connect(m_ui->m_actionDarkTheme, SIGNAL(triggered()), this, SLOT(updateTheme()));
        connect(m_ui->m_actionLightTheme, SIGNAL(triggered()), this, SLOT(updateTheme()));

        // About
        connect(m_ui->m_actionAbout, SIGNAL(triggered()), this, SLOT(about()));
        connect(m_ui->m_actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    }

    void MainWindow::setupTranslator(const QLocale& _locale)
    {
        m_translator.load(_locale, "blokus", "_", ":/translations");
        qApp->installTranslator(&m_translator);
    }

    void MainWindow::updateLanguage()
    {
        qApp->removeTranslator(&m_translator);

        QLocale locale;
        if (m_ui->m_actionEnglish->isChecked())
        {
            locale = QLocale::Language::English;
        }
        else if (m_ui->m_actionFrench->isChecked())
        {
            locale = QLocale::Language::French;
        }

        // Load translation file based on selected language
        setupTranslator(locale);
    }

    void MainWindow::updateTheme()
    {
        THEME_MANAGER.setTheme(m_ui->m_actionDarkTheme->isChecked());
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
}
