#include "theme/ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QSettings>

#include "utils/XmlConfigReader.h"

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

    static const char* getThemeName(bool _darkTheme)
    {
        return _darkTheme ? "dark" : "light";
    }

    ThemeManager::ThemeManager()
        : m_darkTheme(defaultThemeIsDark())
    {
        QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

        setThemeInternal(m_darkTheme);
    }

    void ThemeManager::setThemeInternal(bool _darkTheme)
    {
        m_darkTheme = _darkTheme;

        const char* themeName = getThemeName(_darkTheme);
        // Load application style
        QFile file{ QString { ":/" } + themeName + "/style.qss" };
        file.open(QFile::ReadOnly | QFile::Text);
        qApp->setStyleSheet(file.readAll());

        // Load theme colors
        if (m_colors.find(themeName) == m_colors.cend())
        {
            auto config = XmlConfigReader::loadConfiguration(
                QString{ ":/colors/%1/colors.xml" }.arg(themeName).toStdString(),
                "color");
            m_colors[themeName] = config;
        }
    }

    void ThemeManager::setTheme(bool _darkTheme)
    {
        if (m_darkTheme == _darkTheme)
            return;

        setThemeInternal(_darkTheme);
    }

    QColor ThemeManager::getColor(const QString& _colorName) const
    {
        Configuration colorsConfig = m_colors.at(getThemeName(m_darkTheme));
        auto colorIt = colorsConfig.find(_colorName.toStdString());
        if (colorIt == colorsConfig.cend())
            return {};

        return { colorIt->second.c_str() };
    }

    QString ThemeManager::getResourceName(const QString& _iconName) const
    {
        return QString{ ":/icons/icons/%1/%2" }.arg(getThemeName(m_darkTheme)).arg(_iconName);
    }

    QPixmap ThemeManager::getPixmapResource(const QString& _iconName) const
    {
        QString resourceName = getResourceName(_iconName);
        QPixmap pixmap{ resourceName };
        DEBUG_ASSERT(!pixmap.isNull());
        return pixmap;
    }

    QIcon ThemeManager::getIconResource(const QString& _iconName) const
    {
        QIcon icon;
        QPixmap pixmap{ getPixmapResource(_iconName) };
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
