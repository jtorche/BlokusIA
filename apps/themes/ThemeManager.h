class QString;
class QPixmap;
class QIcon;

namespace blokusUi
{
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
		void setThemeInternal(bool darkTheme);
	
	public:
		void setTheme(bool darkTheme);

        QString getResourceName(const QString& _iconName);
        QPixmap getPixmapResource(const QString& _iconName);
		QIcon getIconResource(const QString& _iconName);

	private:
		bool m_darkTheme;
	};
}
