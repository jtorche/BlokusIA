#pragma once

class QString;
class QPixmap;
class QIcon;

namespace blokusUi
{
    #define THEME_MANAGER blokusUi::ThemeManager::getInstance()

	class ThemeManager
	{
	private:
		ThemeManager();

	public:
		static ThemeManager& getInstance()
		{
			static ThemeManager instance;
			return instance;
		}

	private:
		void setThemeInternal(bool _darkTheme);
	
	public:
        bool isDarkTheme() const { return m_darkTheme; }
        void setTheme(bool _darkTheme);

        QString getResourceName(const QString& _iconName) const;
        QPixmap getPixmapResource(const QString& _iconName) const;
        QIcon getIconResource(const QString& _iconName) const;

	private:
		bool m_darkTheme;
	};
}
