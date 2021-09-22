#pragma once

#include <array>

#include <QBrush>
#include <QGraphicsItem>

#include "IA/BlokusGame.h"

#include "interfaces/IThemeable.h"

namespace blokusUi
{
    class Board : public QGraphicsItem, public IThemeable
    {
    public:
        Board(
            const BlokusIA::Board& _board,
            QGraphicsItem* _parent = nullptr);

        virtual void updateThemedResources() override;

    protected:
        virtual QRectF boundingRect() const override;
        virtual void paint(
            QPainter* _painter,
            const QStyleOptionGraphicsItem* _option,
            QWidget* _widget) override;

    private:
        void assignBrushes();
        void drawBoard(QPainter& _painter) const;

    private:
        BlokusIA::Board m_board;
        QBrush m_boardBrush;
        std::array<QBrush, 4> m_playerBrushes;

        static constexpr f32 BorderWidthRatio = 1/f32(3);
        static u32 ms_scale;
    };
}
