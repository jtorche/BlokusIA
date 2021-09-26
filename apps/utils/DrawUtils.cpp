#include "DrawUtils.h"

#include <QPainter>

#include "game/GameConstants.h"

namespace blokusUI
{
	void DrawUtils::drawTile(QPainter& _painter, const QColor& _color)
	{
        drawTile(_painter, QBrush{ _color });
	}
	
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
}
