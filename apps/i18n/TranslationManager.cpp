#include "i18n/TranslationManager.h"

#include <QCoreApplication>

namespace blokusUi
{
    TranslationManager::TranslationManager()
    {
        setupTranslator(m_locale);
    }

    void TranslationManager::setupTranslator(const QLocale& _locale)
    {
        m_translator.load(_locale, "blokus", "_", ":/translations");
        qApp->installTranslator(&m_translator);
        m_locale = _locale;
    }

    void TranslationManager::setLanguage(const QLocale& _locale)
    {
        if (m_locale == _locale)
            return;

        qApp->removeTranslator(&m_translator);
        setupTranslator(_locale);
    }
}
