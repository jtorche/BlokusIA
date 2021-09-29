#pragma once

#include <QGraphicsItem>

#include "AI/BlokusAI.h"

namespace blokusUI
{
    class PieceGraphicsItem;

    class PlayerPiecesGraphicsItem : public QGraphicsItem
    {
    public:
        PlayerPiecesGraphicsItem(const blokusAI::Slot& _player, QGraphicsItem* _parent = nullptr);

        //virtual void updateThemedResources() override;

        //void setBoard(blokusAI::Board* _board);
        //void addPiece(
        //    const blokusAI::Piece& _piece,
        //    const blokusAI::Slot& _player,
        //    ubyte2 _pos);

    protected:
        virtual QRectF boundingRect() const override;
        virtual void paint(
            QPainter* _painter,
            const QStyleOptionGraphicsItem* _option,
            QWidget* _widget) override;

    //private:
    //    void assignBrushes();
    //    void drawBoard(QPainter& _painter) const;

    //    static constexpr QPointF getBoardOffset();
    //    static constexpr QPointF getTileOffset(u32 _x, u32 _y);

    private:
        //blokusAI::Slot m_player;
        //std::array<PieceGraphicsItem*, blokusAI::BlokusGame::PiecesCount> m_pieces;
        core::flat_hash_map<ubyte, core::flat_hash_set<PieceGraphicsItem*>> m_pieces;
    };
}
