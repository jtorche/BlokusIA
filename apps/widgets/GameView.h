#pragma once

#include "IA/BlokusGame.h"

#include "widgets/BlokusGraphicsView.h"

namespace blokusUi
{
	class BoardGraphicsItem;

    class GameView : public BlokusGraphicsView
	{
	public:
		GameView(QWidget* _parent = nullptr);

		void setBoard(const BlokusIA::Board& _board);

	protected:
		virtual void resizeEvent(QResizeEvent* _event) override;

	private:
		BlokusIA::Board m_board;
		BoardGraphicsItem* m_boardViewer;
	};
}
