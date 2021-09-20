#pragma once

#include "Core/Common.h"

class QColor;
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

        QColor getColor(const QString& _colorName) const;

        QString getResourceName(const QString& _iconName) const;
        QPixmap getPixmapResource(const QString& _iconName) const;
        QIcon getIconResource(const QString& _iconName) const;

	private:
		bool m_darkTheme;

		using ThemeName = std::string;
		using Configuration = core::flat_hash_map<std::string, std::string>;
		core::flat_hash_map<ThemeName, Configuration> m_colors;
	};
}
