#pragma once

#include <QGraphicsView>

#include "AI/BlokusGame.h"

class BoardGraphicsItem;

class BlokusViewer : public QGraphicsView
{
public:
	BlokusViewer(QWidget* _parent = nullptr);

	void setBoard(const blokusAI::Board& _board);

protected:
	virtual void resizeEvent(QResizeEvent* _event) override;

private:
	blokusAI::Board m_board;
	BoardGraphicsItem* m_boardViewer;
};
