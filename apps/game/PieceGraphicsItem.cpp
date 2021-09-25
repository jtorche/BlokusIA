#include "PieceGraphicsItem.h"

#include <QPainter>

#include "theme/ThemeManager.h"

#include "game/GameConstants.h"

namespace blokusUi
{
    PieceGraphicsItem::PieceGraphicsItem(
        const BlokusIA::Piece& _piece,
        const BlokusIA::Slot& _player,
        QGraphicsItem* _parent)
        : QGraphicsItem(_parent)
        , m_player(_player)
        , m_piece(_piece)
    {
        DEBUG_ASSERT(_player != BlokusIA::Slot::Empty);
        assignBrush();
    }

    void PieceGraphicsItem::updateThemedResources()
    {
        assignBrush();
    }

    QRectF PieceGraphicsItem::boundingRect() const
    {
        u32 width = 0;
        u32 height = 0;
        for (ubyte i = 0; i < m_piece.getNumTiles(); ++i)
        {
            BlokusIA::Piece::Tile tile = m_piece.getTile(i);
            width = std::max(width, BlokusIA::Piece::getTileX(tile) + 1);
            height = std::max(height, BlokusIA::Piece::getTileY(tile) + 1);
        }
        return QRectF{ 0, 0, qreal(width * GameConstants::TileSizeScale), qreal(height * GameConstants::TileSizeScale) };
    }

    void PieceGraphicsItem::paint(QPainter* _painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        _painter->setRenderHint(QPainter::Antialiasing);

        // Draw tiles
        for (ubyte i = 0; i < m_piece.getNumTiles(); ++i)
        {
            BlokusIA::Piece::Tile tile = m_piece.getTile(i);            
            drawTile(*_painter, tile);
        }
    }

    static QColor PlayerToColor(const BlokusIA::Slot& _player)
    {
        switch (_player)
        {
        case BlokusIA::Slot::P0:
            return THEME_MANAGER.getColor("player0");

        case BlokusIA::Slot::P1:
            return THEME_MANAGER.getColor("player1");

        case BlokusIA::Slot::P2:
            return THEME_MANAGER.getColor("player2");

        case BlokusIA::Slot::P3:
            return THEME_MANAGER.getColor("player3");
        }

        return {};
    }

    void PieceGraphicsItem::assignBrush()
    {
        m_brush = PlayerToColor(m_player);
    }

    void PieceGraphicsItem::drawTile(QPainter& _painter, const BlokusIA::Piece::Tile& tile) const
    {
        static qreal topAndLeftOutline = 0.04 * GameConstants::TileSizeScale;
        static qreal semiTopAndLeftOutline = topAndLeftOutline / 2;

        static qreal bottomAndRightOutline = 0.08 * GameConstants::TileSizeScale;
        static qreal semiBottomAndRightOutline = bottomAndRightOutline / 2;

        const qreal offsetX = BlokusIA::Piece::getTileX(tile) * GameConstants::TileSizeScale;
        const qreal offsetY = BlokusIA::Piece::getTileY(tile) * GameConstants::TileSizeScale;

        QBrush darkenBrush{ m_brush.color().darker(150) };
        QPen pen{ m_brush, topAndLeftOutline };
        QPen darkenPen{ darkenBrush, bottomAndRightOutline };

        // Draw bottom and right lines
        const qreal darkerOutlineLength = GameConstants::TileSizeScale - semiBottomAndRightOutline;
        QPointF topRight{ darkerOutlineLength + offsetX, semiBottomAndRightOutline + offsetY };
        QPointF bottomLeft{ semiBottomAndRightOutline + offsetX, darkerOutlineLength + offsetY };
        QPointF bottomRight{ darkerOutlineLength + offsetX, darkerOutlineLength + offsetY };

        _painter.setPen(darkenPen);
        _painter.drawLine(topRight, bottomRight);
        _painter.drawLine(bottomRight, bottomLeft);

        // Draw top and left lines
        const qreal outlineLength = GameConstants::TileSizeScale - semiTopAndLeftOutline;
        QPointF topLeft{ semiTopAndLeftOutline + offsetX, semiTopAndLeftOutline + offsetY };
        topRight = { outlineLength - bottomAndRightOutline + offsetX, semiTopAndLeftOutline + offsetY };
        bottomLeft = { semiTopAndLeftOutline + offsetX, outlineLength - bottomAndRightOutline + offsetY };

        _painter.setPen(pen);
        _painter.drawLine(topLeft, topRight);
        _painter.drawLine(topLeft, bottomLeft);

        // Draw center gradient
        topLeft = { topAndLeftOutline + offsetX, topAndLeftOutline + offsetY };
        qreal gradientLength = GameConstants::TileSizeScale - (topAndLeftOutline + bottomAndRightOutline);
        QLinearGradient gradient{ 0, 0, 0, GameConstants::TileSizeScale };
        gradient.setColorAt(0.0, darkenBrush.color());
        gradient.setColorAt(1.0, m_brush.color());
        gradient.setSpread(QGradient::RepeatSpread);

        _painter.setPen(Qt::NoPen);
        _painter.setBrush(gradient);
        _painter.drawRect(topLeft.x(), topLeft.y(), gradientLength, gradientLength);
    }
}
