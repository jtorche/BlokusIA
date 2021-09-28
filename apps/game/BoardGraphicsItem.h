#pragma once

#include <array>

#include <QBrush>
#include <QGraphicsItem>

#include "interfaces/IThemeable.h"

namespace blokusAI
{
    class Board;
    class Piece;
    enum class Slot : ubyte;
}

namespace blokusUI
{
    class BoardGraphicsItem : public QGraphicsItem, public IThemeable
    {
    public:
        BoardGraphicsItem(
            blokusAI::Board* _board,
            QGraphicsItem* _parent = nullptr);

        virtual void updateThemedResources() override;

        void setBoard(blokusAI::Board* _board);
        void addPiece(
            const blokusAI::Piece& _piece,
            const blokusAI::Slot& _player,
            ubyte2 _pos);

    protected:
        virtual QRectF boundingRect() const override;
        virtual void paint(
            QPainter* _painter,
            const QStyleOptionGraphicsItem* _option,
            QWidget* _widget) override;

    private:
        void assignBrushes();

    private:
        blokusAI::Board* m_board;
        QBrush m_boardBrush;
        std::array<QBrush, 4> m_playerBrushes;
    };
}
