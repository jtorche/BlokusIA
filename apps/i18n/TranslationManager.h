#pragma once

#include <QLocale>
#include <QTranslator>

namespace blokusUi
{
    #define TRANSLATION_MANAGER blokusUi::TranslationManager::getInstance()

	class TranslationManager
	{
	private:
		TranslationManager();

	public:
		static TranslationManager& getInstance()
		{
			static TranslationManager instance;
			return instance;
		}

	private:
		void setupTranslator(const QLocale& _locale);
	
	public:
		void setLanguage(const QLocale& _locale);

	private:
		QLocale m_locale{};
		QTranslator m_translator;
	};
}
