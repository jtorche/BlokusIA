#include "game/PlayerPiecesGraphicsItem.h"
//
//#include <QGraphicsScene>
//#include <QPainter>
//
//#include "IA/BlokusGame.h"
//
//#include "theme/ThemeManager.h"
//
//#include "game/GameConstants.h"
//#include "utils/DrawUtils.h"

#include <numeric>

#include "AI/BlokusGameHelpers.h"

#include "game/PieceGraphicsItem.h"

namespace blokusUI
{
    static blokusAI::Piece getMinWidthPiece(const core::flat_hash_set<blokusAI::Piece>& _pieces)
    {
        DEBUG_ASSERT(_pieces.size() > 0);

        blokusAI::Piece minWidthPiece;
        u32 minWidth = std::numeric_limits<u32>::max();
        for (const auto& piece : _pieces)
        {
            u32 width = 0;
            for (ubyte i = 0; i < piece.getNumTiles(); ++i)
            {
                blokusAI::Piece::Tile tile = piece.getTile(i);
                width = std::max(width, blokusAI::Piece::getTileX(tile) + 1);
            }

            if (width < minWidth)
            {
                minWidth = width;
                minWidthPiece = piece;
            }
        }

        return minWidthPiece;
    }

    PlayerPiecesGraphicsItem::PlayerPiecesGraphicsItem(const blokusAI::Slot& _player, QGraphicsItem* _parent)
        : QGraphicsItem(_parent)
        //, m_player(_player)
    {
        //auto pieces = blokusAI::Helpers::getAllPieces();
        blokusAI::PieceSymetries pieceSymetries = blokusAI::Helpers::getAllPieceSymetries();
        /*for (u32 i = 0; i < blokusAI::BlokusGame::PiecesCount; ++i)
        {
            m_pieces[i] = new PieceGraphicsItem(pieces[i], _player, this);
        }*/

        /*for (const auto& piece : pieces)
        {
            m_pieces[piece.getNumTiles()].insert(new PieceGraphicsItem(piece, _player, this));
        }*/

        // Add pieces select the one minimizing the piece width
        for (const auto& pieces : pieceSymetries)
        {
            blokusAI::Piece piece = getMinWidthPiece(pieces);
            m_pieces[piece.getNumTiles()].insert(new PieceGraphicsItem(piece, _player, this));
        }
    }

    //void BoardGraphicsItem::updateThemedResources()
    //{
    //    assignBrushes();
    //}

    QRectF PlayerPiecesGraphicsItem::boundingRect() const
    {
        //TODO
        //constexpr qreal size = (blokusAI::Board::BoardSize + BorderWidthRatio * 2) * GameConstants::TileSizeScale;
        return QRectF{ 0, 0, 10, 10 };
    }

    void PlayerPiecesGraphicsItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
    {
        /*_painter->setRenderHint(QPainter::Antialiasing);

        drawBoard(*_painter);*/
    }

    //void BoardGraphicsItem::assignBrushes()
    //{
    //    m_boardBrush = THEME_MANAGER.getColor("board");

    //    m_playerBrushes[0] = THEME_MANAGER.getColor("player0");
    //    m_playerBrushes[1] = THEME_MANAGER.getColor("player1");
    //    m_playerBrushes[2] = THEME_MANAGER.getColor("player2");
    //    m_playerBrushes[3] = THEME_MANAGER.getColor("player3");
    //}

    //void BoardGraphicsItem::drawBoard(QPainter& _painter) const
    //{
    //    static constexpr qreal boardOutline = 0.03 * GameConstants::TileSizeScale;

    //    static constexpr qreal cornerThickness = BorderWidthRatio * GameConstants::TileSizeScale;
    //    QPointF offset{ getBoardOffset() };

    //    // Translate to real board region
    //    _painter.translate(offset);

    //    // Draw board background
    //    static constexpr qreal boardLength = blokusAI::Board::BoardSize * GameConstants::TileSizeScale;
    //    QPen pen{ m_boardBrush.color().darker(130), boardOutline };
    //    _painter.setPen(pen);
    //    _painter.setBrush(m_boardBrush);
    //    _painter.drawRect(0, 0, boardLength, boardLength);

    //    // Draw board grid outlines
    //    for (u32 i = 0; i < blokusAI::Board::BoardSize; ++i)
    //    {
    //        qreal currentOutlineOffset = i * GameConstants::TileSizeScale;
    //        // Horizontal
    //        _painter.drawLine(QPointF{ 0, currentOutlineOffset }, { boardLength, currentOutlineOffset });
    //        // Vertical
    //        _painter.drawLine(QPointF{ currentOutlineOffset, 0 }, { currentOutlineOffset, boardLength });
    //    }

    //    // Translate back to full board region
    //    _painter.translate(-offset);

    //    // Draw board corners
    //    _painter.setPen(Qt::NoPen);
    //    static constexpr qreal cornerLength = boardLength / 2 + cornerThickness;
    //    static constexpr qreal fullBoardLength = cornerLength * 2;
    //    for (u32 i = 0; i < 4; ++i)
    //    {
    //        QColor playerColor = m_playerBrushes[i].color();

    //        QLinearGradient gradient;
    //        gradient.setStart(0, 0);
    //        gradient.setColorAt(0.0, playerColor);
    //        gradient.setColorAt(0.7, playerColor.darker(110));
    //        gradient.setColorAt(1.0, playerColor.darker(130));

    //        gradient.setFinalStop(0, cornerLength);
    //        _painter.setBrush(gradient);
    //        _painter.drawRect(0, 0, cornerThickness, cornerLength);

    //        gradient.setFinalStop(cornerLength, 0);
    //        _painter.setBrush(gradient);
    //        _painter.drawRect(0, 0, cornerLength, cornerThickness);

    //        _painter.translate(fullBoardLength, 0);
    //        _painter.rotate(90);
    //    }

    //    // Draw pieces tiles
    //    for (u32 j = 0; j < blokusAI::Board::BoardSize; ++j)
    //    {
    //        for (u32 i = 0; i < blokusAI::Board::BoardSize; ++i)
    //        {
    //            blokusAI::Slot slot = m_board->getSlot(i, j);
    //            if (slot == blokusAI::Slot::Empty)
    //                continue;

    //            const QPointF tileOffset{ getTileOffset(i, j) };
    //            _painter.translate(tileOffset);
    //            DrawUtils::drawTile(_painter, m_playerBrushes[u32(slot) - 1]);
    //            _painter.translate(-tileOffset);
    //        }
    //    }
    //}
}
