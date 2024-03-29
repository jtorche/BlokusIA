#pragma once

#include "AI/BlokusGame.h"

#include "widgets/BlokusGraphicsView.h"

namespace blokusUI
{
	class BoardGraphicsItem;

    class GameView : public BlokusGraphicsView
	{
	public:
		GameView(QWidget* _parent = nullptr);

		void setBoard(const blokusAI::Board& _board);

	protected:
		virtual void resizeEvent(QResizeEvent* _event) override;

	private:
		blokusAI::Board m_board;
		BoardGraphicsItem* m_boardViewer;
	};
}
