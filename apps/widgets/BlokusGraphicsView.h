#pragma once

#include <QGraphicsView>

namespace blokusUI
{
    class BlokusGraphicsView : public QGraphicsView
	{
		using QGraphicsView::QGraphicsView;

	protected:
		virtual void changeEvent(QEvent* _event) override;

	private:
        void retranslateItems();
		void updateItemsTheme();
	};
}
