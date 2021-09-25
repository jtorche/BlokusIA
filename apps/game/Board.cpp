#include "Board.h"

#include <QGraphicsScene>
#include <QPainter>

#include "theme/ThemeManager.h"

#include "game/Piece.h"

namespace blokusUi
{
    u32 Board::ms_scale = 50;

    Board::Board(const BlokusIA::Board& _board, QGraphicsItem* _parent)
        : QGraphicsItem(_parent)
        , m_board(_board)
    {
        assignBrushes();
    }

    void Board::updateThemedResources()
    {
        assignBrushes();
    }

    QRectF Board::boundingRect() const
    {
        qreal size = (BlokusIA::Board::BoardSize + BorderWidthRatio * 2) * ms_scale;
        return QRectF{ 0, 0, size, size };
    }

    void Board::paint(QPainter* _painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        _painter->setRenderHint(QPainter::Antialiasing);

        drawBoard(*_painter);
    }

    void Board::assignBrushes()
    {
        m_boardBrush = THEME_MANAGER.getColor("board");

        m_playerBrushes[0] = THEME_MANAGER.getColor("player0");
        m_playerBrushes[1] = THEME_MANAGER.getColor("player1");
        m_playerBrushes[2] = THEME_MANAGER.getColor("player2");
        m_playerBrushes[3] = THEME_MANAGER.getColor("player3");
    }

    void Board::drawBoard(QPainter& _painter) const
    {
        static qreal boardOutline = 0.03 * ms_scale;

        qreal cornerThickness = BorderWidthRatio * ms_scale;
        QPointF offset{ getBoardOffset() };

        // Translate to real board region
        _painter.translate(offset);

        // Draw board background
        qreal boardLength = BlokusIA::Board::BoardSize * ms_scale;
        QPen pen{ m_boardBrush.color().darker(130), boardOutline };
        _painter.setPen(pen);
        _painter.setBrush(m_boardBrush);
        _painter.drawRect(0, 0, boardLength, boardLength);

        // Draw board grid outlines
        for (u32 i = 0; i < BlokusIA::Board::BoardSize; ++i)
        {
            qreal currentOutlineOffset = i * ms_scale;
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

    QPointF Board::getBoardOffset() const
    {
        qreal cornerThickness = BorderWidthRatio * ms_scale;
        return { cornerThickness, cornerThickness };
    }

    void Board::setPiecePosition(Piece& _piece, ubyte2 _pos) const
    {
        QPointF offset{ getBoardOffset() };
        _piece.setPos(offset.x() + _pos.x * ms_scale, offset.y() + _pos.y * ms_scale);
    }

    void Board::addPiece(const BlokusIA::Piece& _piece, const BlokusIA::Slot& _player, ubyte2 _pos)
    {
        m_board.addPiece(_player, _piece, _pos);

        auto piece = new Piece(_piece, _player, ms_scale, this);
        setPiecePosition(*piece, _pos);
        scene()->addItem(piece);
    }
}
