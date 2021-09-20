#include "Piece.h"

#include <QPainter>

#include "themes/ThemeManager.h"

namespace blokusUi
{
    u32 Piece::ms_scale = 50;

    Piece::Piece(const BlokusIA::Piece& _piece, const BlokusIA::Slot& _player, QGraphicsItem* _parent)
        : QGraphicsItem(_parent)
        , m_player(_player)
        , m_piece(_piece)
    {
        DEBUG_ASSERT(_player != BlokusIA::Slot::Empty);
    }

    QRectF Piece::boundingRect() const
    {
        u32 width = 0;
        u32 height = 0;
        for (ubyte i = 0; i < m_piece.getNumTiles(); ++i)
        {
            BlokusIA::Piece::Tile tile = m_piece.getTile(i);
            width = std::max(width, BlokusIA::Piece::getTileX(tile) + 1);
            height = std::max(height, BlokusIA::Piece::getTileY(tile) + 1);
        }
        return QRectF{ 0, 0, qreal(width * ms_scale), qreal(height * ms_scale) };
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

    void Piece::paint(QPainter* _painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        _painter->setRenderHint(QPainter::Antialiasing);

        QBrush brush{ PlayerToColor(m_player) };
        QBrush darkenBrush{ brush.color().darker(150) };

        for (ubyte i = 0; i < m_piece.getNumTiles(); ++i)
        {
            BlokusIA::Piece::Tile tile = m_piece.getTile(i);            
            drawTile(*_painter, tile, brush, darkenBrush);
        }
    }

    void Piece::drawTile(QPainter& _painter, const BlokusIA::Piece::Tile& tile, const QBrush& brush, const QBrush& darkenBrush) const
    {
        static qreal topAndLeftOutline = 0.04 * ms_scale;
        static qreal semiTopAndLeftOutline = topAndLeftOutline / 2;

        static qreal bottomAndRightOutline = 0.08 * ms_scale;
        static qreal semiBottomAndRightOutline = bottomAndRightOutline / 2;

        const qreal offsetX = BlokusIA::Piece::getTileX(tile) * ms_scale;
        const qreal offsetY = BlokusIA::Piece::getTileY(tile) * ms_scale;

        QPen pen{ brush, topAndLeftOutline };
        QPen darkenPen{ darkenBrush, bottomAndRightOutline };

        // Draw bottom and right lines
        const qreal darkerOutlineLength = ms_scale - semiBottomAndRightOutline;
        QPointF topRight{ darkerOutlineLength + offsetX, semiBottomAndRightOutline + offsetY };
        QPointF bottomLeft{ semiBottomAndRightOutline + offsetX, darkerOutlineLength + offsetY };
        QPointF bottomRight{ darkerOutlineLength + offsetX, darkerOutlineLength + offsetY };

        _painter.setPen(darkenPen);
        _painter.drawLine(topRight, bottomRight);
        _painter.drawLine(bottomRight, bottomLeft);

        // Draw top and left lines
        const qreal outlineLength = ms_scale - semiTopAndLeftOutline;
        QPointF topLeft{ semiTopAndLeftOutline + offsetX, semiTopAndLeftOutline + offsetY };
        topRight = { outlineLength - bottomAndRightOutline + offsetX, semiTopAndLeftOutline + offsetY };
        bottomLeft = { semiTopAndLeftOutline + offsetX, outlineLength - bottomAndRightOutline + offsetY };

        _painter.setPen(pen);
        _painter.drawLine(topLeft, topRight);
        _painter.drawLine(topLeft, bottomLeft);

        // Draw center gradient
        topLeft = { topAndLeftOutline + offsetX, topAndLeftOutline + offsetY };
        qreal gradientLength = ms_scale - (topAndLeftOutline + bottomAndRightOutline);
        QLinearGradient gradient{ 0, 0, 0, qreal(ms_scale) };
        gradient.setColorAt(0.0, darkenBrush.color());
        gradient.setColorAt(1.0, brush.color());
        gradient.setSpread(QGradient::RepeatSpread);

        _painter.setPen(Qt::NoPen);
        _painter.setBrush(gradient);
        _painter.drawRect(topLeft.x(), topLeft.y(), gradientLength, gradientLength);
    }
}
