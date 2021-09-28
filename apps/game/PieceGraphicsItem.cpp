#include "game/PieceGraphicsItem.h"

#include <QPainter>

#include "UI/game/GameConstants.h"
#include "UI/utils/DrawUtils.h"

#include "theme/ThemeManager.h"

namespace blokusUI
{
    PieceGraphicsItem::PieceGraphicsItem(
        const blokusAI::Piece& _piece,
        const blokusAI::Slot& _player,
        QGraphicsItem* _parent)
        : QGraphicsItem(_parent)
        , m_player(_player)
        , m_piece(_piece)
    {
        DEBUG_ASSERT(_player != blokusAI::Slot::Empty);
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
            blokusAI::Piece::Tile tile = m_piece.getTile(i);
            width = std::max(width, blokusAI::Piece::getTileX(tile) + 1);
            height = std::max(height, blokusAI::Piece::getTileY(tile) + 1);
        }
        return QRectF{ 0, 0, qreal(width * GameConstants::TileSizeScale), qreal(height * GameConstants::TileSizeScale) };
    }

    void PieceGraphicsItem::paint(QPainter* _painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        _painter->setRenderHint(QPainter::Antialiasing);

        // Draw tiles
        for (ubyte i = 0; i < m_piece.getNumTiles(); ++i)
        {
            blokusAI::Piece::Tile tile = m_piece.getTile(i);            
            QPointF offset{
                qreal(blokusAI::Piece::getTileX(tile) * GameConstants::TileSizeScale),
                qreal(blokusAI::Piece::getTileY(tile) * GameConstants::TileSizeScale) };

            _painter->translate(offset);
            DrawUtils::drawTile(*_painter, m_brush);
            _painter->translate(-offset);
        }
    }

    static QColor PlayerToColor(const blokusAI::Slot& _player)
    {
        switch (_player)
        {
        case blokusAI::Slot::P0:
            return THEME_MANAGER.getColor("player0");

        case blokusAI::Slot::P1:
            return THEME_MANAGER.getColor("player1");

        case blokusAI::Slot::P2:
            return THEME_MANAGER.getColor("player2");

        case blokusAI::Slot::P3:
            return THEME_MANAGER.getColor("player3");
        }

        return {};
    }

    void PieceGraphicsItem::assignBrush()
    {
        m_brush = PlayerToColor(m_player);
    }
}
