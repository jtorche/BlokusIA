#include "UI/utils/DrawUtils.h"

#include <QPainter>

#include "AI/BlokusGame.h"
#include "UI/game/GameConstants.h"

namespace blokusUI
{
	void DrawUtils::drawTile(QPainter& _painter, const QBrush& _brush)
	{
        static constexpr qreal topAndLeftOutline = 0.04 * GameConstants::TileSizeScale;
        static constexpr qreal semiTopAndLeftOutline = topAndLeftOutline / 2;

        static constexpr qreal bottomAndRightOutline = 0.08 * GameConstants::TileSizeScale;
        static constexpr qreal semiBottomAndRightOutline = bottomAndRightOutline / 2;

        QBrush darkenBrush{ _brush.color().darker(150) };
        QPen pen{ _brush, topAndLeftOutline };
        QPen darkenPen{ darkenBrush, bottomAndRightOutline };

        // Draw bottom and right lines
        static constexpr qreal darkerOutlineLength = GameConstants::TileSizeScale - semiBottomAndRightOutline;
        QPointF topRight{ darkerOutlineLength, semiBottomAndRightOutline };
        QPointF bottomLeft{ semiBottomAndRightOutline, darkerOutlineLength };
        QPointF bottomRight{ darkerOutlineLength, darkerOutlineLength };

        _painter.setPen(darkenPen);
        _painter.drawLine(topRight, bottomRight);
        _painter.drawLine(bottomRight, bottomLeft);

        // Draw top and left lines
        static constexpr qreal outlineLength = GameConstants::TileSizeScale - semiTopAndLeftOutline;
        QPointF topLeft{ semiTopAndLeftOutline, semiTopAndLeftOutline };
        topRight = { outlineLength - bottomAndRightOutline, semiTopAndLeftOutline };
        bottomLeft = { semiTopAndLeftOutline, outlineLength - bottomAndRightOutline };

        _painter.setPen(pen);
        _painter.drawLine(topLeft, topRight);
        _painter.drawLine(topLeft, bottomLeft);

        // Draw center gradient
        topLeft = { topAndLeftOutline, topAndLeftOutline };
        static constexpr qreal gradientLength = GameConstants::TileSizeScale - (topAndLeftOutline + bottomAndRightOutline);
        QLinearGradient gradient{ 0, 0, 0, GameConstants::TileSizeScale };
        gradient.setColorAt(0.0, darkenBrush.color());
        gradient.setColorAt(1.0, _brush.color());
        gradient.setSpread(QGradient::RepeatSpread);

        _painter.setPen(Qt::NoPen);
        _painter.setBrush(gradient);
        _painter.drawRect(topLeft.x(), topLeft.y(), gradientLength, gradientLength);
	}

    void DrawUtils::drawBoard(
        QPainter& _painter,
        const blokusAI::Board& _board,
        const QBrush& _boardBrush,
        const std::array<QBrush, 4>& _playerBrushes)
    {
        static constexpr qreal boardOutline = 0.03 * GameConstants::TileSizeScale;

        static constexpr qreal cornerThickness = BoardBorderWidthRatio * GameConstants::TileSizeScale;
        QPointF offset{ getBoardOffset() };

        // Translate to real board region
        _painter.translate(offset);

        // Draw board background
        static constexpr qreal boardLength = blokusAI::Board::BoardSize * GameConstants::TileSizeScale;
        QPen pen{ _boardBrush.color().darker(130), boardOutline };
        _painter.setPen(pen);
        _painter.setBrush(_boardBrush);
        _painter.drawRect(0, 0, boardLength, boardLength);

        // Draw board grid outlines
        for (u32 i = 0; i < blokusAI::Board::BoardSize; ++i)
        {
            qreal currentOutlineOffset = i * GameConstants::TileSizeScale;
            // Horizontal
            _painter.drawLine(QPointF{ 0, currentOutlineOffset }, { boardLength, currentOutlineOffset });
            // Vertical
            _painter.drawLine(QPointF{ currentOutlineOffset, 0 }, { currentOutlineOffset, boardLength });
        }

        // Translate back to full board region
        _painter.translate(-offset);

        // Draw board corners
        _painter.setPen(Qt::NoPen);
        static constexpr qreal cornerLength = boardLength / 2 + cornerThickness;
        static constexpr qreal fullBoardLength = cornerLength * 2;
        for (u32 i = 0; i < 4; ++i)
        {
            QColor playerColor = _playerBrushes[i].color();

            QLinearGradient gradient;
            gradient.setStart(0, 0);
            gradient.setColorAt(0.0, playerColor);
            gradient.setColorAt(0.7, playerColor.darker(110));
            gradient.setColorAt(1.0, playerColor.darker(130));

            gradient.setFinalStop(0, cornerLength);
            _painter.setBrush(gradient);
            _painter.drawRect(0, 0, cornerThickness, cornerLength);

            gradient.setFinalStop(cornerLength, 0);
            _painter.setBrush(gradient);
            _painter.drawRect(0, 0, cornerLength, cornerThickness);

            _painter.translate(fullBoardLength, 0);
            _painter.rotate(90);
        }

        // Draw pieces tiles
        for (u32 j = 0; j < blokusAI::Board::BoardSize; ++j)
        {
            for (u32 i = 0; i < blokusAI::Board::BoardSize; ++i)
            {
                blokusAI::Slot slot = _board.getSlot(i, j);
                if (slot == blokusAI::Slot::Empty)
                    continue;

                const QPointF tileOffset{ getTileOffset(i, j) };
                _painter.translate(tileOffset);
                DrawUtils::drawTile(_painter, _playerBrushes[u32(slot) - 1]);
                _painter.translate(-tileOffset);
            }
        }
    }
}
