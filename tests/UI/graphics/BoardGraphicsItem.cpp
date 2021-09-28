#include "graphics/BoardGraphicsItem.h"

#include <QPainter>

#include "AI/BlokusGame.h"
#include "UI/utils/DrawUtils.h"

BoardGraphicsItem::BoardGraphicsItem(QGraphicsItem* _parent)
    : QGraphicsItem(_parent)
    , m_board{ nullptr }
    , m_boardBrush{ QColor{ "#b4b4b4" } }
    , m_playerBrushes{ QColor{ "#3581d8" }, QColor{ "#23aa27"}, QColor{ "#d82e3f" }, QColor{ "#ffe135" } }
{
}

QRectF BoardGraphicsItem::boundingRect() const
{
    return blokusUI::DrawUtils::getBoardBoundingRect();
}

void BoardGraphicsItem::setBoard(blokusAI::Board* _board)
{
    m_board = _board;

    update();
}

void BoardGraphicsItem::paint(QPainter* _painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (m_board == nullptr)
        return;

    _painter->setRenderHint(QPainter::Antialiasing);

    blokusUI::DrawUtils::drawBoard(*_painter, *m_board, m_boardBrush, m_playerBrushes);
}
