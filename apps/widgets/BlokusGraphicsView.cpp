#include "widgets/BlokusGraphicsView.h"

#include <QEvent>
#include <QGraphicsItem>

#include "Core/Algorithms.h"

#include "interfaces/IThemeable.h"
#include "interfaces/ITranslatable.h"

namespace blokusUi
{
    void BlokusGraphicsView::changeEvent(QEvent* _event)
    {
        if (_event->type() == QEvent::LanguageChange)
        {
            retranslateItems();
        }
        else if (_event->type() == QEvent::StyleChange)
        {
            updateItemsTheme();
        }

        QGraphicsView::changeEvent(_event);
    }

    void BlokusGraphicsView::retranslateItems()
    {
        if (scene() == nullptr)
            return;

        core::dynamic_for_each<ITranslatable*>(scene()->items(), [](ITranslatable* _translatable)
        {
            _translatable->retranslate();
        });
    }

    void BlokusGraphicsView::updateItemsTheme()
    {
        if (scene() == nullptr)
            return;
        
        core::dynamic_for_each<IThemeable*>(scene()->items(), [](IThemeable* _themeable)
        {
            _themeable->updateThemedResources();
        });
    }
}
