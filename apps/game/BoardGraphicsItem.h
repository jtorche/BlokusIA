#pragma once

#include <array>

#include <QBrush>
#include <QGraphicsItem>

#include "IA/BlokusGame.h"

#include "interfaces/IThemeable.h"

namespace blokusUi
{
    class PieceGraphicsItem;

    class BoardGraphicsItem : public QGraphicsItem, public IThemeable
    {
    public:
        BoardGraphicsItem(
            BlokusIA::Board* _board,
            QGraphicsItem* _parent = nullptr);

        virtual void updateThemedResources() override;

        void setBoard(BlokusIA::Board* _board);
        void addPiece(
            const BlokusIA::Piece& _piece,
            const BlokusIA::Slot& _player,
            ubyte2 _pos);

    protected:
        virtual QRectF boundingRect() const override;
        virtual void paint(
            QPainter* _painter,
            const QStyleOptionGraphicsItem* _option,
            QWidget* _widget) override;

    private:
        void assignBrushes();
        void drawBoard(QPainter& _painter) const;

        QPointF getBoardOffset() const;
        void setPiecePosition(PieceGraphicsItem& _piece, ubyte2 _pos) const;

    private:
        BlokusIA::Board* m_board;
        QBrush m_boardBrush;
        std::array<QBrush, 4> m_playerBrushes;

        static constexpr f32 BorderWidthRatio = 1/f32(3);
    };
}
