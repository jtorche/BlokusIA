#include "themes/ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QSettings>

namespace blokusUi
{
    static bool windowsIsInDarkTheme()
    {
        QSettings settings{ "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat };
        return settings.value("AppsUseLightTheme", 1).toInt() == 0;
    }

    static bool defaultThemeIsDark()
    {
        return windowsIsInDarkTheme();
    }

    static char* getThemeName(bool darkTheme)
    {
        return darkTheme ? "dark" : "light";
    }

    ThemeManager::ThemeManager()
        : m_darkTheme(defaultThemeIsDark())
    {
        setThemeInternal(m_darkTheme);
    }

    void ThemeManager::setThemeInternal(bool darkTheme)
    {
        QFile file{ QString { ":/" } + getThemeName(darkTheme) + "/style.qss" };
        file.open(QFile::ReadOnly | QFile::Text);
        qApp->setStyleSheet(file.readAll());

        m_darkTheme = darkTheme;
    }

    void ThemeManager::setTheme(bool darkTheme)
    {
        if (m_darkTheme == darkTheme)
            return;

        setThemeInternal(darkTheme);
    }

    QString ThemeManager::getResourceName(const QString& _iconName)
    {
        //TODO
        return QString{ ":/icons/%1/%2" }.arg(getThemeName(m_darkTheme)).arg(_iconName);
    }

    QPixmap ThemeManager::getPixmapResource(const QString& _iconName)
    {
        QString resourceName = getResourceName(_iconName);
        QPixmap pixmap = QPixmap{ resourceName };
        DEBUG_ASSERT(!pixmap.isNull());
        return pixmap;
    }

    QIcon ThemeManager::getIconResource(const QString& _iconName)
    {
        QIcon icon;
        QPixmap pixmap = getPixmapResource(_iconName);
        icon.addPixmap(pixmap);
        if (m_darkTheme)
        {
            // Automatic disabled icon is no good for dark
            // paint transparent black to get disabled look
            QPainter p{ &pixmap };
            p.fillRect(pixmap.rect(), QColor{ 48, 47, 47, 128 });
            icon.addPixmap(pixmap, QIcon::Disabled);
        }
        return icon;
    }
}
