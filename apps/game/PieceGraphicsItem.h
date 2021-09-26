#pragma once

#include <QBrush>
#include <QGraphicsItem>

#include "AI/BlokusGame.h"

#include "interfaces/IThemeable.h"

namespace blokusUI
{
    class PieceGraphicsItem : public QGraphicsItem, public IThemeable
    {
    public:
        PieceGraphicsItem(
            const blokusAI::Piece& _piece,
            const blokusAI::Slot& _player,
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

    private:
        blokusAI::Slot m_player;
        blokusAI::Piece m_piece;
        QBrush m_brush;
    };
}
