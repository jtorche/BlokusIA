#pragma once

#include <array>

#include <QBrush>
#include <QGraphicsItem>

namespace blokusAI
{
    class Board;
}

class BoardGraphicsItem : public QGraphicsItem
{
public:
    BoardGraphicsItem(QGraphicsItem* _parent = nullptr);

    void setBoard(blokusAI::Board* _board);

protected:
    virtual QRectF boundingRect() const override;
    virtual void paint(
        QPainter* _painter,
        const QStyleOptionGraphicsItem* _option,
        QWidget* _widget) override;

private:
    blokusAI::Board* m_board;
    QBrush m_boardBrush;
    std::array<QBrush, 4> m_playerBrushes;
};
