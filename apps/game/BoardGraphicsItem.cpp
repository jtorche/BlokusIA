#include "game/BoardGraphicsItem.h"

#include <QPainter>

#include "AI/BlokusGame.h"
#include "UI/utils/DrawUtils.h"

#include "theme/ThemeManager.h"

namespace blokusUI
{
    BoardGraphicsItem::BoardGraphicsItem(blokusAI::Board* _board, QGraphicsItem* _parent)
        : QGraphicsItem(_parent)
        , m_board(_board)
    {
        assignBrushes();
    }

    void BoardGraphicsItem::updateThemedResources()
    {
        assignBrushes();
    }

    QRectF BoardGraphicsItem::boundingRect() const
    {
        return DrawUtils::getBoardBoundingRect();
    }

    void BoardGraphicsItem::setBoard(blokusAI::Board* _board)
    {
        DEBUG_ASSERT(_board != nullptr);
        m_board = _board;

        update();
    }

    void BoardGraphicsItem::addPiece(const blokusAI::Piece& _piece, const blokusAI::Slot& _player, ubyte2 _pos)
    {
        m_board->addPiece(_player, _piece, _pos);

        update();
    }

    void BoardGraphicsItem::paint(QPainter* _painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        _painter->setRenderHint(QPainter::Antialiasing);

        DrawUtils::drawBoard(*_painter, *m_board, m_boardBrush, m_playerBrushes);
    }

    void BoardGraphicsItem::assignBrushes()
    {
        m_boardBrush = THEME_MANAGER.getColor("board");

        m_playerBrushes[0] = THEME_MANAGER.getColor("player0");
        m_playerBrushes[1] = THEME_MANAGER.getColor("player1");
        m_playerBrushes[2] = THEME_MANAGER.getColor("player2");
        m_playerBrushes[3] = THEME_MANAGER.getColor("player3");
    }
}
