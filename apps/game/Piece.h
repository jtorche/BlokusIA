#pragma once

#include <QBrush>
#include <QGraphicsItem>

#include "IA/BlokusGame.h"

#include "interfaces/IThemeable.h"

namespace blokusUi
{
    class Piece : public QGraphicsItem, public IThemeable
    {
    public:
        Piece(
            const BlokusIA::Piece& _piece,
            const BlokusIA::Slot& _player,
            f32 _tileSize,
            QGraphicsItem* _parent = nullptr);

        virtual void updateThemedResources() override;

    protected:
        virtual QRectF boundingRect() const override;
        virtual void paint(
            QPainter* _painter,
            const QStyleOptionGraphicsItem* _option,
            QWidget* _widget) override;

    private:
        void assignBrush();
        void drawTile(QPainter& _painter, const BlokusIA::Piece::Tile& tile) const;

    private:
        BlokusIA::Slot m_player;
        BlokusIA::Piece m_piece;
        f32 m_tileSize;
        QBrush m_brush;
    };
}
