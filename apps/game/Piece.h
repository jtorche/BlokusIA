#pragma once

#include <QGraphicsItem>

#include "IA/BlokusGame.h"

namespace blokusUi
{
    class Piece : public QGraphicsItem
    {
    public:
        Piece(const BlokusIA::Piece& _piece, const BlokusIA::Slot& _player, QGraphicsItem* _parent = nullptr);

    protected:
        virtual QRectF boundingRect() const override;
        virtual void paint(
            QPainter* _painter,
            const QStyleOptionGraphicsItem* _option,
            QWidget* _widget) override;

    private:
        void drawTile(QPainter& _painter, const BlokusIA::Piece::Tile& tile, const QBrush& brush, const QBrush& darkenBrush) const;

    private:
        BlokusIA::Slot m_player;
        BlokusIA::Piece m_piece;
        static u32 ms_scale;
    };
}
