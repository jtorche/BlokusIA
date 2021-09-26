#include "PieceGraphicsItem.h"

#include <QPainter>

#include "theme/ThemeManager.h"

#include "game/GameConstants.h"
#include "utils/DrawUtils.h"

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
            QPointF offset{
                qreal(BlokusIA::Piece::getTileX(tile) * GameConstants::TileSizeScale),
                qreal(BlokusIA::Piece::getTileY(tile) * GameConstants::TileSizeScale) };

            _painter->translate(offset);
            DrawUtils::drawTile(*_painter, m_brush);
            _painter->translate(-offset);
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
}
