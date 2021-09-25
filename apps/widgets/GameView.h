#pragma once

#include "widgets/BlokusGraphicsView.h"

namespace blokusUi
{
	class BoardGraphicsItem;

    class GameView : public BlokusGraphicsView
	{
	public:
		GameView(QWidget* _parent = nullptr);

	protected:
		virtual void resizeEvent(QResizeEvent* _event) override;

	private:
		BoardGraphicsItem* m_board;
	};
}
