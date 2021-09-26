#include "BoardGraphicsItem.h"

#include <QGraphicsScene>
#include <QPainter>

#include "theme/ThemeManager.h"

#include "game/GameConstants.h"
#include "game/PieceGraphicsItem.h"

namespace blokusUi
{
    BoardGraphicsItem::BoardGraphicsItem(BlokusIA::Board* _board, QGraphicsItem* _parent)
        : QGraphicsItem(_parent)
        , m_board(_board)
    {
        assignBrushes();
    }

    void BoardGraphicsItem::updateThemedResources()
    {
        assignBrushes();
    }

    QRectF BoardGraphicsItem::boundingRect() const
    {
        qreal size = (BlokusIA::Board::BoardSize + BorderWidthRatio * 2) * GameConstants::TileSizeScale;
        return QRectF{ 0, 0, size, size };
    }

    void BoardGraphicsItem::setBoard(BlokusIA::Board* _board)
    {
        m_board = _board;

        for (const auto& item : childItems())
        {
            scene()->removeItem(item);
            delete item;
        }
    }

    QPointF BoardGraphicsItem::getBoardOffset() const
    {
        qreal cornerThickness = BorderWidthRatio * GameConstants::TileSizeScale;
        return { cornerThickness, cornerThickness };
    }

    void BoardGraphicsItem::setPiecePosition(PieceGraphicsItem& _piece, ubyte2 _pos) const
    {
        QPointF offset{ getBoardOffset() };
        _piece.setPos(
            offset.x() + _pos.x * GameConstants::TileSizeScale,
            offset.y() + _pos.y * GameConstants::TileSizeScale);
    }

    void BoardGraphicsItem::addPiece(const BlokusIA::Piece& _piece, const BlokusIA::Slot& _player, ubyte2 _pos)
    {
        m_board->addPiece(_player, _piece, _pos);

        auto piece = new PieceGraphicsItem(_piece, _player, this);
        setPiecePosition(*piece, _pos);
        scene()->addItem(piece);
    }

    void BoardGraphicsItem::paint(QPainter* _painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        _painter->setRenderHint(QPainter::Antialiasing);

        drawBoard(*_painter);
    }

    void BoardGraphicsItem::assignBrushes()
    {
        m_boardBrush = THEME_MANAGER.getColor("board");

        m_playerBrushes[0] = THEME_MANAGER.getColor("player0");
        m_playerBrushes[1] = THEME_MANAGER.getColor("player1");
        m_playerBrushes[2] = THEME_MANAGER.getColor("player2");
        m_playerBrushes[3] = THEME_MANAGER.getColor("player3");
    }

    void BoardGraphicsItem::drawBoard(QPainter& _painter) const
    {
        static qreal boardOutline = 0.03 * GameConstants::TileSizeScale;

        qreal cornerThickness = BorderWidthRatio * GameConstants::TileSizeScale;
        QPointF offset{ getBoardOffset() };

        // Translate to real board region
        _painter.translate(offset);

        // Draw board background
        qreal boardLength = BlokusIA::Board::BoardSize * GameConstants::TileSizeScale;
        QPen pen{ m_boardBrush.color().darker(130), boardOutline };
        _painter.setPen(pen);
        _painter.setBrush(m_boardBrush);
        _painter.drawRect(0, 0, boardLength, boardLength);

        // Draw board grid outlines
        for (u32 i = 0; i < BlokusIA::Board::BoardSize; ++i)
        {
            qreal currentOutlineOffset = i * GameConstants::TileSizeScale;
            // Horizontal
            _painter.drawLine(QPointF{ 0, currentOutlineOffset }, { boardLength, currentOutlineOffset });
            // Vertical
            _painter.drawLine(QPointF{ currentOutlineOffset, 0 }, { currentOutlineOffset, boardLength });
        }

        // Translate back to full board region
        _painter.translate(-offset);

        // Draw board corners
        _painter.setPen(Qt::NoPen);
        qreal cornerLength = boardLength / 2 + cornerThickness;
        qreal fullBoardLength = cornerLength * 2;
        for (u32 i = 0; i < 4; ++i)
        {
            QColor playerColor = m_playerBrushes[i].color();

            QLinearGradient gradient;
            gradient.setStart(0, 0);
            gradient.setColorAt(0.0, playerColor);
            gradient.setColorAt(0.7, playerColor.darker(110));
            gradient.setColorAt(1.0, playerColor.darker(130));

            gradient.setFinalStop(0, cornerLength);
            _painter.setBrush(gradient);
            _painter.drawRect(0, 0, cornerThickness, cornerLength);

            gradient.setFinalStop(cornerLength, 0);
            _painter.setBrush(gradient);
            _painter.drawRect(0, 0, cornerLength, cornerThickness);

            _painter.translate(fullBoardLength, 0);
            _painter.rotate(90);
        }
    }
}
