#pragma once

#include <QPointF>
#include <QRectF>

class QBrush;
class QPainter;

namespace blokusAI
{
	class Board;
}

namespace blokusUI
{
    struct DrawUtils
	{
		static void drawTile(QPainter& _painter, const QBrush& _brush);
		
		static constexpr QRectF getBoardBoundingRect();
		static constexpr QPointF getBoardOffset();
		static constexpr QPointF getTileOffset(u32 _x, u32 _y);
		static void drawBoard(
			QPainter& _painter,
			const blokusAI::Board& _board,
			const QBrush& _boardBrush,
			const std::array<QBrush, 4>& _playerBrushes);

	private:
		static constexpr f32 BoardBorderWidthRatio = 1 / f32(3);
	};
}

#include "UI/utils/DrawUtils_inl.h"
