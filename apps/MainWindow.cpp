#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QActionGroup>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMessageBox>

#include "IA/BlokusGameHelpers.h"

#include "i18n/TranslationManager.h"
#include "theme/ThemeManager.h"

#include "widgets/BlokusGraphicsView.h"

#include "game/Piece.h"

namespace blokusUi
{
    MainWindow::MainWindow(QWidget* _parent)
        : QMainWindow(_parent)
        , m_ui(new Ui::MainWindow)
    {
        m_ui->setupUi(this);

        setupActions();
        setupConnections();

        auto view = new BlokusGraphicsView(m_ui->m_centralwidget);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        
        view->setFixedSize(1200, 1000);
        auto scene = new QGraphicsScene(view);
        scene->setSceneRect(0, 0, 1200, 1000);
        view->setScene(scene);
        setGeometry(0, 0, 1200, 1200);
        u32 x = 10;
        u32 y = 10;
        auto pieces = BlokusIA::Helpers::getAllPieces();
        BlokusIA::Slot slot = BlokusIA::Slot::P0;
        for (const auto& p : pieces)
        {
            auto piece = new Piece(p, slot);
            
            if (slot == BlokusIA::Slot::P0)
                slot = BlokusIA::Slot::P1;
            else if (slot == BlokusIA::Slot::P1)
                slot = BlokusIA::Slot::P2;
            else if (slot == BlokusIA::Slot::P2)
                slot = BlokusIA::Slot::P3;
            else if (slot == BlokusIA::Slot::P3)
                slot = BlokusIA::Slot::P0;
            
            piece->setPos(x, y);
            scene->addItem(piece);

            x += 210;
            if (x + 210 > 1200)
            {
                y += 160;
                x = 10;
            }
        }
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
        else if (_event->type() == QEvent::StyleChange)
        {
            setupThemedResources();
        }

        QMainWindow::changeEvent(_event);
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

        setupThemedResources();
    }

    void MainWindow::setupConnections() const
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


    void MainWindow::setupThemedResources() const
    {
        m_ui->m_actionQuit->setIcon(THEME_MANAGER.getIconResource("quit"));
        m_ui->m_actionDarkTheme->setIcon(THEME_MANAGER.getIconResource("dark"));
        m_ui->m_actionLightTheme->setIcon(THEME_MANAGER.getIconResource("light"));
    }

    void MainWindow::updateLanguage() const
    {
        QLocale locale;
        if (m_ui->m_actionEnglish->isChecked())
        {
            locale = QLocale::Language::English;
        }
        else if (m_ui->m_actionFrench->isChecked())
        {
            locale = QLocale::Language::French;
        }

        TRANSLATION_MANAGER.setLanguage(locale);
    }

    void MainWindow::updateTheme() const
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
